#include "Core_PhysicsImpactSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

UCore_PhysicsImpactSystem::UCore_PhysicsImpactSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default impact thresholds
    MinImpactVelocity = 100.0f;
    MaxImpactVelocity = 2000.0f;
    ImpactDamageMultiplier = 1.0f;
    DebrisLifetime = 5.0f;
    MaxDebrisCount = 20;
    
    // Initialize surface type mappings
    InitializeSurfaceTypeMappings();
}

void UCore_PhysicsImpactSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Register for collision events
    if (AActor* Owner = GetOwner())
    {
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* Primitive : PrimitiveComponents)
        {
            if (Primitive && Primitive->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                Primitive->OnComponentHit.AddDynamic(this, &UCore_PhysicsImpactSystem::OnComponentHit);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsImpactSystem initialized for actor: %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
}

void UCore_PhysicsImpactSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update active impact effects
    UpdateActiveImpacts(DeltaTime);
    
    // Clean up expired debris
    CleanupExpiredDebris(DeltaTime);
}

void UCore_PhysicsImpactSystem::OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
                                               UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
                                               const FHitResult& Hit)
{
    if (!HitComp || !OtherActor || !OtherComp)
    {
        return;
    }
    
    // Calculate impact velocity
    FVector RelativeVelocity = FVector::ZeroVector;
    if (HitComp->GetBodyInstance())
    {
        RelativeVelocity = HitComp->GetBodyInstance()->GetUnrealWorldVelocity();
    }
    
    if (OtherComp->GetBodyInstance())
    {
        RelativeVelocity -= OtherComp->GetBodyInstance()->GetUnrealWorldVelocity();
    }
    
    float ImpactSpeed = RelativeVelocity.Size();
    
    // Check if impact is significant enough
    if (ImpactSpeed < MinImpactVelocity)
    {
        return;
    }
    
    // Create impact data
    FCore_ImpactData ImpactData;
    ImpactData.Location = Hit.Location;
    ImpactData.Normal = Hit.Normal;
    ImpactData.Velocity = RelativeVelocity;
    ImpactData.ImpactSpeed = ImpactSpeed;
    ImpactData.SurfaceType = GetSurfaceType(Hit);
    ImpactData.HitActor = OtherActor;
    ImpactData.HitComponent = OtherComp;
    
    // Process the impact
    ProcessImpact(ImpactData);
}

void UCore_PhysicsImpactSystem::ProcessImpact(const FCore_ImpactData& ImpactData)
{
    // Calculate impact intensity (0.0 to 1.0)
    float ImpactIntensity = FMath::Clamp(
        (ImpactData.ImpactSpeed - MinImpactVelocity) / (MaxImpactVelocity - MinImpactVelocity),
        0.0f, 1.0f
    );
    
    // Spawn visual effects
    SpawnImpactEffects(ImpactData, ImpactIntensity);
    
    // Play audio effects
    PlayImpactAudio(ImpactData, ImpactIntensity);
    
    // Create debris if impact is strong enough
    if (ImpactIntensity > 0.3f)
    {
        CreateImpactDebris(ImpactData, ImpactIntensity);
    }
    
    // Apply damage if applicable
    ApplyImpactDamage(ImpactData, ImpactIntensity);
    
    // Log significant impacts
    if (ImpactIntensity > 0.5f)
    {
        UE_LOG(LogTemp, Warning, TEXT("High impact detected: Speed=%.1f, Intensity=%.2f, Location=%s"),
               ImpactData.ImpactSpeed, ImpactIntensity, *ImpactData.Location.ToString());
    }
}

void UCore_PhysicsImpactSystem::SpawnImpactEffects(const FCore_ImpactData& ImpactData, float Intensity)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get surface-specific effect
    FCore_SurfaceImpactEffect* SurfaceEffect = SurfaceEffects.Find(ImpactData.SurfaceType);
    if (!SurfaceEffect)
    {
        SurfaceEffect = SurfaceEffects.Find(ECore_SurfaceType::Default);
    }
    
    if (!SurfaceEffect)
    {
        return;
    }
    
    // Spawn particle effect
    if (SurfaceEffect->ParticleEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            SurfaceEffect->ParticleEffect,
            ImpactData.Location,
            ImpactData.Normal.Rotation(),
            FVector(Intensity),
            true
        );
    }
    
    // Spawn Niagara effect if available
    if (SurfaceEffect->NiagaraEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            SurfaceEffect->NiagaraEffect,
            ImpactData.Location,
            ImpactData.Normal.Rotation(),
            FVector(Intensity),
            true
        );
    }
}

void UCore_PhysicsImpactSystem::PlayImpactAudio(const FCore_ImpactData& ImpactData, float Intensity)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get surface-specific effect
    FCore_SurfaceImpactEffect* SurfaceEffect = SurfaceEffects.Find(ImpactData.SurfaceType);
    if (!SurfaceEffect || !SurfaceEffect->ImpactSound)
    {
        return;
    }
    
    // Calculate volume and pitch based on intensity
    float Volume = FMath::Lerp(0.3f, 1.0f, Intensity);
    float Pitch = FMath::Lerp(0.8f, 1.2f, Intensity);
    
    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        SurfaceEffect->ImpactSound,
        ImpactData.Location,
        Volume,
        Pitch
    );
}

void UCore_PhysicsImpactSystem::CreateImpactDebris(const FCore_ImpactData& ImpactData, float Intensity)
{
    if (!GetWorld() || ActiveDebris.Num() >= MaxDebrisCount)
    {
        return;
    }
    
    // Get surface-specific effect
    FCore_SurfaceImpactEffect* SurfaceEffect = SurfaceEffects.Find(ImpactData.SurfaceType);
    if (!SurfaceEffect || !SurfaceEffect->DebrisMesh)
    {
        return;
    }
    
    // Calculate number of debris pieces based on intensity
    int32 DebrisCount = FMath::RoundToInt(FMath::Lerp(2.0f, 8.0f, Intensity));
    DebrisCount = FMath::Min(DebrisCount, MaxDebrisCount - ActiveDebris.Num());
    
    for (int32 i = 0; i < DebrisCount; i++)
    {
        // Create debris actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AActor* DebrisActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnParams);
        if (!DebrisActor)
        {
            continue;
        }
        
        // Add static mesh component
        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(DebrisActor);
        MeshComp->SetStaticMesh(SurfaceEffect->DebrisMesh);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetSimulatePhysics(true);
        MeshComp->SetCollisionObjectType(ECC_WorldDynamic);
        DebrisActor->SetRootComponent(MeshComp);
        
        // Position debris near impact point
        FVector DebrisLocation = ImpactData.Location + FMath::VRand() * 50.0f;
        DebrisActor->SetActorLocation(DebrisLocation);
        
        // Apply random impulse
        FVector Impulse = (ImpactData.Normal + FMath::VRand() * 0.5f).GetSafeNormal() * 
                         FMath::RandRange(100.0f, 500.0f) * Intensity;
        MeshComp->AddImpulse(Impulse);
        
        // Add to active debris list
        FCore_DebrisData DebrisData;
        DebrisData.DebrisActor = DebrisActor;
        DebrisData.SpawnTime = GetWorld()->GetTimeSeconds();
        DebrisData.Lifetime = DebrisLifetime;
        ActiveDebris.Add(DebrisData);
    }
}

void UCore_PhysicsImpactSystem::ApplyImpactDamage(const FCore_ImpactData& ImpactData, float Intensity)
{
    if (!ImpactData.HitActor || ImpactDamageMultiplier <= 0.0f)
    {
        return;
    }
    
    // Calculate damage based on impact intensity
    float Damage = Intensity * ImpactDamageMultiplier * 100.0f; // Base damage of 100
    
    // Apply damage if the actor can receive it
    if (ImpactData.HitActor->CanBeDamaged())
    {
        UGameplayStatics::ApplyPointDamage(
            ImpactData.HitActor,
            Damage,
            ImpactData.Location,
            ImpactData.Location,
            nullptr,
            GetOwner(),
            UDamageType::StaticClass()
        );
        
        UE_LOG(LogTemp, Log, TEXT("Applied impact damage: %.1f to %s"), 
               Damage, *ImpactData.HitActor->GetName());
    }
}

void UCore_PhysicsImpactSystem::UpdateActiveImpacts(float DeltaTime)
{
    // Update any ongoing impact effects here
    // This could include fading particle effects, cooling down surfaces, etc.
}

void UCore_PhysicsImpactSystem::CleanupExpiredDebris(float DeltaTime)
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = ActiveDebris.Num() - 1; i >= 0; i--)
    {
        FCore_DebrisData& DebrisData = ActiveDebris[i];
        
        if (CurrentTime - DebrisData.SpawnTime > DebrisData.Lifetime)
        {
            // Destroy expired debris
            if (IsValid(DebrisData.DebrisActor))
            {
                DebrisData.DebrisActor->Destroy();
            }
            
            ActiveDebris.RemoveAt(i);
        }
    }
}

ECore_SurfaceType UCore_PhysicsImpactSystem::GetSurfaceType(const FHitResult& Hit)
{
    if (!Hit.PhysMaterial.IsValid())
    {
        return ECore_SurfaceType::Default;
    }
    
    // Map physical material to surface type
    EPhysicalSurface PhysSurface = Hit.PhysMaterial->SurfaceType;
    
    ECore_SurfaceType* MappedType = PhysicalSurfaceMapping.Find(PhysSurface);
    return MappedType ? *MappedType : ECore_SurfaceType::Default;
}

void UCore_PhysicsImpactSystem::InitializeSurfaceTypeMappings()
{
    // Map UE5 physical surface types to our custom surface types
    PhysicalSurfaceMapping.Add(SurfaceType1, ECore_SurfaceType::Stone);    // Stone
    PhysicalSurfaceMapping.Add(SurfaceType2, ECore_SurfaceType::Wood);     // Wood
    PhysicalSurfaceMapping.Add(SurfaceType3, ECore_SurfaceType::Metal);    // Metal
    PhysicalSurfaceMapping.Add(SurfaceType4, ECore_SurfaceType::Flesh);    // Flesh
    PhysicalSurfaceMapping.Add(SurfaceType5, ECore_SurfaceType::Dirt);     // Dirt
    PhysicalSurfaceMapping.Add(SurfaceType6, ECore_SurfaceType::Water);    // Water
    PhysicalSurfaceMapping.Add(SurfaceType_Default, ECore_SurfaceType::Default);
}

void UCore_PhysicsImpactSystem::RegisterSurfaceEffect(ECore_SurfaceType SurfaceType, 
                                                       const FCore_SurfaceImpactEffect& Effect)
{
    SurfaceEffects.Add(SurfaceType, Effect);
    
    UE_LOG(LogTemp, Log, TEXT("Registered surface effect for type: %d"), (int32)SurfaceType);
}

void UCore_PhysicsImpactSystem::SetImpactThresholds(float MinVelocity, float MaxVelocity)
{
    MinImpactVelocity = FMath::Max(0.0f, MinVelocity);
    MaxImpactVelocity = FMath::Max(MinImpactVelocity + 1.0f, MaxVelocity);
    
    UE_LOG(LogTemp, Log, TEXT("Impact thresholds updated: Min=%.1f, Max=%.1f"), 
           MinImpactVelocity, MaxImpactVelocity);
}

void UCore_PhysicsImpactSystem::SetDebrisSettings(float Lifetime, int32 MaxCount)
{
    DebrisLifetime = FMath::Max(0.1f, Lifetime);
    MaxDebrisCount = FMath::Max(1, MaxCount);
    
    UE_LOG(LogTemp, Log, TEXT("Debris settings updated: Lifetime=%.1f, MaxCount=%d"), 
           DebrisLifetime, MaxDebrisCount);
}

void UCore_PhysicsImpactSystem::ClearAllDebris()
{
    for (const FCore_DebrisData& DebrisData : ActiveDebris)
    {
        if (IsValid(DebrisData.DebrisActor))
        {
            DebrisData.DebrisActor->Destroy();
        }
    }
    
    ActiveDebris.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("All debris cleared"));
}

int32 UCore_PhysicsImpactSystem::GetActiveDebrisCount() const
{
    return ActiveDebris.Num();
}

TArray<AActor*> UCore_PhysicsImpactSystem::GetActiveDebrisActors() const
{
    TArray<AActor*> DebrisActors;
    
    for (const FCore_DebrisData& DebrisData : ActiveDebris)
    {
        if (IsValid(DebrisData.DebrisActor))
        {
            DebrisActors.Add(DebrisData.DebrisActor);
        }
    }
    
    return DebrisActors;
}