#include "Core_ImpactSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

UCore_ImpactSystem::UCore_ImpactSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default impact thresholds
    MinImpactVelocity = 300.0f;
    MaxImpactVelocity = 2000.0f;
    
    // Default material impact settings
    DefaultImpactSettings.DamageMultiplier = 1.0f;
    DefaultImpactSettings.SoundVolume = 0.8f;
    DefaultImpactSettings.ParticleScale = 1.0f;
    DefaultImpactSettings.bCanCauseDamage = true;
    DefaultImpactSettings.bPlaySound = true;
    DefaultImpactSettings.bSpawnParticles = true;
}

void UCore_ImpactSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize impact system
    InitializeImpactSystem();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_ImpactSystem: Impact system initialized"));
}

void UCore_ImpactSystem::InitializeImpactSystem()
{
    // Setup default material impact mappings
    SetupDefaultMaterialImpacts();
    
    // Register for collision events
    RegisterCollisionCallbacks();
}

void UCore_ImpactSystem::SetupDefaultMaterialImpacts()
{
    // Stone impact settings
    FCore_ImpactSettings StoneSettings;
    StoneSettings.DamageMultiplier = 1.5f;
    StoneSettings.SoundVolume = 1.0f;
    StoneSettings.ParticleScale = 1.2f;
    StoneSettings.bCanCauseDamage = true;
    StoneSettings.bPlaySound = true;
    StoneSettings.bSpawnParticles = true;
    
    // Wood impact settings
    FCore_ImpactSettings WoodSettings;
    WoodSettings.DamageMultiplier = 0.8f;
    WoodSettings.SoundVolume = 0.6f;
    WoodSettings.ParticleScale = 0.8f;
    WoodSettings.bCanCauseDamage = true;
    WoodSettings.bPlaySound = true;
    WoodSettings.bSpawnParticles = true;
    
    // Flesh impact settings
    FCore_ImpactSettings FleshSettings;
    FleshSettings.DamageMultiplier = 2.0f;
    FleshSettings.SoundVolume = 0.5f;
    FleshSettings.ParticleScale = 0.6f;
    FleshSettings.bCanCauseDamage = true;
    FleshSettings.bPlaySound = true;
    FleshSettings.bSpawnParticles = true;
    
    // Store material impact settings
    MaterialImpactMap.Add(TEXT("Stone"), StoneSettings);
    MaterialImpactMap.Add(TEXT("Wood"), WoodSettings);
    MaterialImpactMap.Add(TEXT("Flesh"), FleshSettings);
}

void UCore_ImpactSystem::RegisterCollisionCallbacks()
{
    // Get owner actor
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_ImpactSystem: No owner actor found"));
        return;
    }
    
    // Find all primitive components and bind collision events
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component && Component->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
        {
            Component->OnComponentHit.AddDynamic(this, &UCore_ImpactSystem::OnComponentHit);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_ImpactSystem: Registered collision callbacks for %d components"), PrimitiveComponents.Num());
}

void UCore_ImpactSystem::OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HitComp || !OtherActor || !OtherComp)
    {
        return;
    }
    
    // Calculate impact velocity
    float ImpactVelocity = NormalImpulse.Size();
    
    // Check if impact is strong enough
    if (ImpactVelocity < MinImpactVelocity)
    {
        return;
    }
    
    // Process the impact
    ProcessImpact(Hit, ImpactVelocity, HitComp, OtherActor, OtherComp);
}

void UCore_ImpactSystem::ProcessImpact(const FHitResult& HitResult, float ImpactVelocity, UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent)
{
    // Determine impact material
    FString MaterialType = DetermineMaterialType(HitResult, OtherComponent);
    
    // Get impact settings for this material
    FCore_ImpactSettings* ImpactSettings = MaterialImpactMap.Find(MaterialType);
    if (!ImpactSettings)
    {
        ImpactSettings = &DefaultImpactSettings;
    }
    
    // Calculate normalized impact strength (0.0 to 1.0)
    float ImpactStrength = FMath::Clamp((ImpactVelocity - MinImpactVelocity) / (MaxImpactVelocity - MinImpactVelocity), 0.0f, 1.0f);
    
    // Apply impact effects
    if (ImpactSettings->bPlaySound)
    {
        PlayImpactSound(HitResult.Location, MaterialType, ImpactStrength, ImpactSettings->SoundVolume);
    }
    
    if (ImpactSettings->bSpawnParticles)
    {
        SpawnImpactParticles(HitResult.Location, HitResult.Normal, MaterialType, ImpactStrength, ImpactSettings->ParticleScale);
    }
    
    if (ImpactSettings->bCanCauseDamage)
    {
        ApplyImpactDamage(OtherActor, ImpactVelocity, ImpactSettings->DamageMultiplier);
    }
    
    // Log impact for debugging
    UE_LOG(LogTemp, Log, TEXT("Core_ImpactSystem: Impact processed - Material: %s, Velocity: %.2f, Strength: %.2f"), *MaterialType, ImpactVelocity, ImpactStrength);
}

FString UCore_ImpactSystem::DetermineMaterialType(const FHitResult& HitResult, UPrimitiveComponent* Component)
{
    if (!Component)
    {
        return TEXT("Default");
    }
    
    // Check if component has a physics material
    if (UPhysicalMaterial* PhysMat = Component->GetBodyInstance()->GetSimplePhysicalMaterial())
    {
        FString MaterialName = PhysMat->GetName();
        
        // Map physics material names to our material types
        if (MaterialName.Contains(TEXT("Stone")) || MaterialName.Contains(TEXT("Rock")))
        {
            return TEXT("Stone");
        }
        else if (MaterialName.Contains(TEXT("Wood")) || MaterialName.Contains(TEXT("Tree")))
        {
            return TEXT("Wood");
        }
        else if (MaterialName.Contains(TEXT("Flesh")) || MaterialName.Contains(TEXT("Body")))
        {
            return TEXT("Flesh");
        }
    }
    
    // Fallback to actor class-based detection
    if (AActor* Owner = Component->GetOwner())
    {
        FString ActorName = Owner->GetClass()->GetName();
        
        if (ActorName.Contains(TEXT("Rock")) || ActorName.Contains(TEXT("Stone")))
        {
            return TEXT("Stone");
        }
        else if (ActorName.Contains(TEXT("Tree")) || ActorName.Contains(TEXT("Wood")))
        {
            return TEXT("Wood");
        }
        else if (ActorName.Contains(TEXT("Character")) || ActorName.Contains(TEXT("Pawn")))
        {
            return TEXT("Flesh");
        }
    }
    
    return TEXT("Default");
}

void UCore_ImpactSystem::PlayImpactSound(const FVector& Location, const FString& MaterialType, float ImpactStrength, float VolumeMultiplier)
{
    // For now, use a simple sound playing approach
    // In a full implementation, you would load appropriate sound assets based on material type
    
    if (UWorld* World = GetWorld())
    {
        // Calculate volume based on impact strength
        float Volume = ImpactStrength * VolumeMultiplier;
        
        // Play a generic impact sound
        // Note: In production, you would have different sounds for different materials
        UGameplayStatics::PlaySoundAtLocation(
            World,
            nullptr, // Would be a loaded sound asset
            Location,
            Volume,
            1.0f, // Pitch
            0.0f, // Start time
            nullptr, // Attenuation settings
            nullptr, // Concurrency settings
            GetOwner()
        );
        
        UE_LOG(LogTemp, Log, TEXT("Core_ImpactSystem: Impact sound played - Material: %s, Volume: %.2f"), *MaterialType, Volume);
    }
}

void UCore_ImpactSystem::SpawnImpactParticles(const FVector& Location, const FVector& Normal, const FString& MaterialType, float ImpactStrength, float ScaleMultiplier)
{
    if (UWorld* World = GetWorld())
    {
        // Calculate particle scale based on impact strength
        float ParticleScale = ImpactStrength * ScaleMultiplier;
        
        // For now, we'll just log the particle spawn
        // In production, you would spawn appropriate particle systems based on material type
        UE_LOG(LogTemp, Log, TEXT("Core_ImpactSystem: Impact particles spawned - Material: %s, Scale: %.2f"), *MaterialType, ParticleScale);
        
        // Example of how you would spawn Niagara particles:
        // UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        //     World,
        //     ImpactParticleSystem, // Would be a loaded Niagara system
        //     Location,
        //     FRotator::ZeroRotator,
        //     FVector(ParticleScale),
        //     true,
        //     true,
        //     ENCPoolMethod::None,
        //     true
        // );
    }
}

void UCore_ImpactSystem::ApplyImpactDamage(AActor* TargetActor, float ImpactVelocity, float DamageMultiplier)
{
    if (!TargetActor)
    {
        return;
    }
    
    // Calculate damage based on impact velocity
    float BaseDamage = (ImpactVelocity - MinImpactVelocity) / 100.0f; // Scale damage
    float FinalDamage = BaseDamage * DamageMultiplier;
    
    // Only apply damage if it's significant
    if (FinalDamage > 1.0f)
    {
        // Apply damage to the target actor
        UGameplayStatics::ApplyPointDamage(
            TargetActor,
            FinalDamage,
            TargetActor->GetActorLocation(),
            FHitResult(),
            nullptr, // Controller
            GetOwner(), // Damage causer
            UDamageType::StaticClass()
        );
        
        UE_LOG(LogTemp, Log, TEXT("Core_ImpactSystem: Applied %.2f damage to %s"), FinalDamage, *TargetActor->GetName());
    }
}

void UCore_ImpactSystem::SetMaterialImpactSettings(const FString& MaterialType, const FCore_ImpactSettings& Settings)
{
    MaterialImpactMap.Add(MaterialType, Settings);
    UE_LOG(LogTemp, Log, TEXT("Core_ImpactSystem: Material impact settings updated for %s"), *MaterialType);
}

FCore_ImpactSettings UCore_ImpactSystem::GetMaterialImpactSettings(const FString& MaterialType) const
{
    if (const FCore_ImpactSettings* Settings = MaterialImpactMap.Find(MaterialType))
    {
        return *Settings;
    }
    return DefaultImpactSettings;
}

void UCore_ImpactSystem::SetImpactVelocityRange(float MinVelocity, float MaxVelocity)
{
    MinImpactVelocity = MinVelocity;
    MaxImpactVelocity = MaxVelocity;
    UE_LOG(LogTemp, Log, TEXT("Core_ImpactSystem: Impact velocity range set to %.2f - %.2f"), MinVelocity, MaxVelocity);
}