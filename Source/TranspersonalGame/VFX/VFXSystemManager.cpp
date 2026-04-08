#include "VFXSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UVFXSystemManager::UVFXSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS tick for performance
}

void UVFXSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeVFXLibrary();
}

void UVFXSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Performance management
    CleanupInactiveVFX();
    
    // LOD updates
    if (GetWorld()->GetTimeSeconds() - LastLODUpdateTime > LODUpdateInterval)
    {
        if (APawn* ViewerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            UpdateLODSystem(ViewerPawn->GetActorLocation());
        }
        LastLODUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

UNiagaraComponent* UVFXSystemManager::SpawnVFX(const FString& EffectName, FVector Location, FRotator Rotation, AActor* AttachToActor)
{
    // Check if we're at max VFX limit
    if (ActiveVFXComponents.Num() >= MaxActiveVFX)
    {
        // Remove lowest priority VFX
        UNiagaraComponent* LowestPriorityVFX = nullptr;
        EVFXPriority LowestPriority = EVFXPriority::Critical;
        
        for (UNiagaraComponent* VFX : ActiveVFXComponents)
        {
            if (VFX && VFX->IsValidLowLevel())
            {
                // Get priority from VFX name or component tags
                EVFXPriority VFXPriority = EVFXPriority::Background; // Default to lowest
                
                if (static_cast<uint8>(VFXPriority) >= static_cast<uint8>(LowestPriority))
                {
                    LowestPriority = VFXPriority;
                    LowestPriorityVFX = VFX;
                }
            }
        }
        
        if (LowestPriorityVFX)
        {
            StopVFX(LowestPriorityVFX);
        }
    }
    
    // Find effect in library
    if (FVFXEffectData* EffectData = VFXLibrary.Find(EffectName))
    {
        if (EffectData->NiagaraSystem)
        {
            UNiagaraComponent* NewVFX = nullptr;
            
            if (AttachToActor)
            {
                NewVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
                    EffectData->NiagaraSystem,
                    AttachToActor->GetRootComponent(),
                    NAME_None,
                    Location,
                    Rotation,
                    EAttachLocation::KeepWorldPosition,
                    true
                );
            }
            else
            {
                NewVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                    GetWorld(),
                    EffectData->NiagaraSystem,
                    Location,
                    Rotation
                );
            }
            
            if (NewVFX)
            {
                ActiveVFXComponents.Add(NewVFX);
                
                // Apply initial LOD based on distance
                if (APawn* ViewerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
                {
                    float Distance = FVector::Dist(ViewerPawn->GetActorLocation(), Location);
                    ApplyLODToVFX(NewVFX, Distance);
                }
                
                return NewVFX;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Effect '%s' not found in library"), *EffectName);
    return nullptr;
}

void UVFXSystemManager::StopVFX(UNiagaraComponent* VFXComponent)
{
    if (VFXComponent && VFXComponent->IsValidLowLevel())
    {
        VFXComponent->Deactivate();
        ActiveVFXComponents.Remove(VFXComponent);
    }
}

void UVFXSystemManager::StopAllVFXByCategory(EVFXCategory Category)
{
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* VFX = ActiveVFXComponents[i];
        if (VFX && VFX->IsValidLowLevel())
        {
            // Check category via component tags or name
            // This would need to be expanded based on how we tag VFX
            StopVFX(VFX);
        }
    }
}

void UVFXSystemManager::SetGlobalVFXQuality(float QualityLevel)
{
    GlobalQualityMultiplier = FMath::Clamp(QualityLevel, 0.1f, 1.0f);
    
    // Update all active VFX
    for (UNiagaraComponent* VFX : ActiveVFXComponents)
    {
        if (VFX && VFX->IsValidLowLevel())
        {
            // Apply quality multiplier to relevant parameters
            VFX->SetFloatParameter(TEXT("QualityMultiplier"), GlobalQualityMultiplier);
        }
    }
}

void UVFXSystemManager::UpdateLODSystem(FVector ViewerLocation)
{
    for (UNiagaraComponent* VFX : ActiveVFXComponents)
    {
        if (VFX && VFX->IsValidLowLevel())
        {
            float Distance = FVector::Dist(ViewerLocation, VFX->GetComponentLocation());
            ApplyLODToVFX(VFX, Distance);
        }
    }
}

int32 UVFXSystemManager::GetActiveVFXCount() const
{
    return ActiveVFXComponents.Num();
}

void UVFXSystemManager::PlayCreatureBreathingEffect(AActor* Creature, float IntensityMultiplier)
{
    if (!Creature) return;
    
    FVector BreathLocation = Creature->GetActorLocation() + FVector(0, 0, 100); // Adjust based on creature
    SpawnVFX(TEXT("CreatureBreath"), BreathLocation, FRotator::ZeroRotator, Creature);
}

void UVFXSystemManager::PlayCreatureFootstepEffect(FVector Location, float CreatureSize)
{
    FString FootstepType = TEXT("SmallFootstep");
    if (CreatureSize > 500.0f) FootstepType = TEXT("LargeFootstep");
    else if (CreatureSize > 200.0f) FootstepType = TEXT("MediumFootstep");
    
    SpawnVFX(FootstepType, Location);
}

void UVFXSystemManager::PlayDomesticationProgressEffect(AActor* Creature, float TrustLevel)
{
    if (!Creature) return;
    
    FString EffectName = TEXT("TrustBuilding");
    if (TrustLevel > 0.8f) EffectName = TEXT("FullyTrusted");
    else if (TrustLevel > 0.5f) EffectName = TEXT("Friendly");
    else if (TrustLevel > 0.2f) EffectName = TEXT("Cautious");
    
    SpawnVFX(EffectName, Creature->GetActorLocation(), FRotator::ZeroRotator, Creature);
}

void UVFXSystemManager::PlayWeatherEffect(const FString& WeatherType, float Intensity)
{
    FString EffectName = FString::Printf(TEXT("Weather_%s"), *WeatherType);
    
    // Spawn at multiple locations for coverage
    if (APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        FVector PlayerLocation = Player->GetActorLocation();
        
        // Create weather effects in a radius around player
        for (int32 i = 0; i < 4; i++)
        {
            float Angle = (i * 90.0f) * PI / 180.0f;
            FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0) * 2000.0f;
            SpawnVFX(EffectName, PlayerLocation + Offset);
        }
    }
}

void UVFXSystemManager::PlayVegetationDisturbance(FVector Location, float Radius)
{
    SpawnVFX(TEXT("VegetationRustle"), Location);
    
    // Add particle effects for leaves/debris
    SpawnVFX(TEXT("FallingLeaves"), Location + FVector(0, 0, 200));
}

void UVFXSystemManager::PlayImpactEffect(FVector Location, FVector Normal, const FString& SurfaceType)
{
    FString EffectName = FString::Printf(TEXT("Impact_%s"), *SurfaceType);
    FRotator ImpactRotation = Normal.Rotation();
    
    SpawnVFX(EffectName, Location, ImpactRotation);
}

void UVFXSystemManager::PlayBloodEffect(FVector Location, FVector Direction, float Intensity)
{
    FString EffectName = TEXT("BloodSplatter");
    if (Intensity > 0.7f) EffectName = TEXT("BloodSplatterHeavy");
    
    FRotator BloodRotation = Direction.Rotation();
    SpawnVFX(EffectName, Location, BloodRotation);
}

void UVFXSystemManager::InitializeVFXLibrary()
{
    // Initialize with default effect data
    // These would be loaded from data assets in a real implementation
    
    FVFXEffectData CreatureBreath;
    CreatureBreath.Priority = EVFXPriority::Medium;
    CreatureBreath.Category = EVFXCategory::Creature;
    CreatureBreath.MaxDistance = 1000.0f;
    VFXLibrary.Add(TEXT("CreatureBreath"), CreatureBreath);
    
    FVFXEffectData SmallFootstep;
    SmallFootstep.Priority = EVFXPriority::Low;
    SmallFootstep.Category = EVFXCategory::Creature;
    SmallFootstep.MaxDistance = 500.0f;
    VFXLibrary.Add(TEXT("SmallFootstep"), SmallFootstep);
    
    FVFXEffectData LargeFootstep;
    LargeFootstep.Priority = EVFXPriority::High;
    LargeFootstep.Category = EVFXCategory::Creature;
    LargeFootstep.MaxDistance = 2000.0f;
    VFXLibrary.Add(TEXT("LargeFootstep"), LargeFootstep);
    
    FVFXEffectData BloodSplatter;
    BloodSplatter.Priority = EVFXPriority::High;
    BloodSplatter.Category = EVFXCategory::Combat;
    BloodSplatter.MaxDistance = 1500.0f;
    VFXLibrary.Add(TEXT("BloodSplatter"), BloodSplatter);
    
    // Add more effects as needed...
}

void UVFXSystemManager::CleanupInactiveVFX()
{
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* VFX = ActiveVFXComponents[i];
        if (!VFX || !VFX->IsValidLowLevel() || !VFX->IsActive())
        {
            ActiveVFXComponents.RemoveAt(i);
        }
    }
}

void UVFXSystemManager::ApplyLODToVFX(UNiagaraComponent* VFXComponent, float Distance)
{
    if (!VFXComponent) return;
    
    float QualityMultiplier = CalculateQualityMultiplier(Distance, EVFXPriority::Medium);
    
    // Apply LOD parameters to Niagara system
    VFXComponent->SetFloatParameter(TEXT("LOD_Quality"), QualityMultiplier);
    VFXComponent->SetFloatParameter(TEXT("LOD_ParticleCount"), QualityMultiplier);
    VFXComponent->SetFloatParameter(TEXT("LOD_UpdateRate"), QualityMultiplier);
    
    // Disable VFX if too far
    if (Distance > LowQualityDistance)
    {
        VFXComponent->SetVisibility(false);
    }
    else
    {
        VFXComponent->SetVisibility(true);
    }
}

float UVFXSystemManager::CalculateQualityMultiplier(float Distance, EVFXPriority Priority)
{
    float BaseMultiplier = 1.0f;
    
    if (Distance <= HighQualityDistance)
    {
        BaseMultiplier = 1.0f;
    }
    else if (Distance <= MediumQualityDistance)
    {
        BaseMultiplier = 0.7f;
    }
    else if (Distance <= LowQualityDistance)
    {
        BaseMultiplier = 0.4f;
    }
    else
    {
        BaseMultiplier = 0.1f;
    }
    
    // Priority modifier
    float PriorityMultiplier = 1.0f;
    switch (Priority)
    {
        case EVFXPriority::Critical:
            PriorityMultiplier = 1.2f;
            break;
        case EVFXPriority::High:
            PriorityMultiplier = 1.0f;
            break;
        case EVFXPriority::Medium:
            PriorityMultiplier = 0.8f;
            break;
        case EVFXPriority::Low:
            PriorityMultiplier = 0.6f;
            break;
        case EVFXPriority::Background:
            PriorityMultiplier = 0.4f;
            break;
    }
    
    return BaseMultiplier * PriorityMultiplier * GlobalQualityMultiplier;
}