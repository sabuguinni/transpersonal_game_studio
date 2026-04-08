#include "VFXArchitecture.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

UVFXManagerComponent::UVFXManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance

    MaxActiveEffects = 50;
    LODUpdateInterval = 0.5f; // Update LOD twice per second
    CurrentLODLevel = EVFXLODLevel::High;
    CurrentEnvironmentalTension = 0.0f;
    LastLODUpdateTime = 0.0f;
}

void UVFXManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize VFX database with default effects
    InitializeDefaultVFXDatabase();
}

void UVFXManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update LOD levels based on performance and distance
    if (GetWorld()->GetTimeSeconds() - LastLODUpdateTime > LODUpdateInterval)
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            UpdateVFXLODBasedOnDistance(PlayerPawn);
        }
        LastLODUpdateTime = GetWorld()->GetTimeSeconds();
    }

    // Cleanup inactive effects
    CleanupInactiveEffects();
}

void UVFXManagerComponent::PlayVFXEffect(const FString& EffectName, FVector Location, FRotator Rotation)
{
    // Find effect definition
    FVFXDefinition* EffectDef = VFXDatabase.FindByPredicate([&EffectName](const FVFXDefinition& Def)
    {
        return Def.EffectName == EffectName;
    });

    if (!EffectDef)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect not found: %s"), *EffectName);
        return;
    }

    // Check if we're at max active effects
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        // Remove oldest effect to make room
        auto OldestEffect = ActiveEffects.begin();
        if (OldestEffect->Value && IsValid(OldestEffect->Value))
        {
            OldestEffect->Value->DestroyComponent();
        }
        ActiveEffects.Remove(OldestEffect->Key);
    }

    // Get appropriate Niagara system for current LOD
    UNiagaraSystem* NiagaraSystem = GetNiagaraSystemForLOD(*EffectDef, CurrentLODLevel);
    
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Niagara System found for effect: %s at LOD: %d"), 
               *EffectName, (int32)CurrentLODLevel);
        return;
    }

    // Check distance requirements
    if (EffectDef->bRequiresPlayerProximity)
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            float DistanceToPlayer = FVector::Dist(Location, PlayerPawn->GetActorLocation());
            if (DistanceToPlayer > EffectDef->PlayerProximityRadius)
            {
                return; // Too far from player, don't spawn
            }
        }
    }

    // Spawn the effect
    UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        NiagaraSystem,
        Location,
        Rotation
    );

    if (SpawnedEffect)
    {
        // Apply environmental tension modifications
        ModifyEffectForTension(SpawnedEffect, *EffectDef);
        
        // Store reference
        ActiveEffects.Add(EffectName + FString::Printf(TEXT("_%d"), FMath::RandRange(1000, 9999)), SpawnedEffect);
        
        UE_LOG(LogTemp, Log, TEXT("VFX Effect spawned: %s at %s"), *EffectName, *Location.ToString());
    }
}

void UVFXManagerComponent::StopVFXEffect(const FString& EffectName)
{
    for (auto It = ActiveEffects.CreateIterator(); It; ++It)
    {
        if (It->Key.Contains(EffectName))
        {
            if (It->Value && IsValid(It->Value))
            {
                It->Value->DestroyComponent();
            }
            It.RemoveCurrent();
        }
    }
}

void UVFXManagerComponent::SetVFXLODLevel(EVFXLODLevel NewLODLevel)
{
    if (CurrentLODLevel != NewLODLevel)
    {
        CurrentLODLevel = NewLODLevel;
        
        // Update all active effects to new LOD level
        for (auto& EffectPair : ActiveEffects)
        {
            if (EffectPair.Value && IsValid(EffectPair.Value))
            {
                // Find the effect definition and update the system
                FString BaseEffectName = EffectPair.Key.Left(EffectPair.Key.Find(TEXT("_")));
                FVFXDefinition* EffectDef = VFXDatabase.FindByPredicate([&BaseEffectName](const FVFXDefinition& Def)
                {
                    return Def.EffectName == BaseEffectName;
                });

                if (EffectDef)
                {
                    UNiagaraSystem* NewSystem = GetNiagaraSystemForLOD(*EffectDef, NewLODLevel);
                    if (NewSystem)
                    {
                        EffectPair.Value->SetAsset(NewSystem);
                    }
                }
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("VFX LOD Level changed to: %d"), (int32)NewLODLevel);
    }
}

void UVFXManagerComponent::UpdateVFXLODBasedOnDistance(APawn* PlayerPawn)
{
    if (!PlayerPawn) return;

    // Calculate average distance to active effects
    float TotalDistance = 0.0f;
    int32 ValidEffects = 0;

    for (const auto& EffectPair : ActiveEffects)
    {
        if (EffectPair.Value && IsValid(EffectPair.Value))
        {
            float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), EffectPair.Value->GetComponentLocation());
            TotalDistance += Distance;
            ValidEffects++;
        }
    }

    if (ValidEffects > 0)
    {
        float AverageDistance = TotalDistance / ValidEffects;
        EVFXLODLevel NewLODLevel = CalculateLODLevel(AverageDistance);
        SetVFXLODLevel(NewLODLevel);
    }
}

void UVFXManagerComponent::SetMaxActiveEffects(int32 MaxEffects)
{
    MaxActiveEffects = FMath::Max(1, MaxEffects);
    
    // If we're over the new limit, remove excess effects
    while (ActiveEffects.Num() > MaxActiveEffects)
    {
        auto OldestEffect = ActiveEffects.begin();
        if (OldestEffect->Value && IsValid(OldestEffect->Value))
        {
            OldestEffect->Value->DestroyComponent();
        }
        ActiveEffects.Remove(OldestEffect->Key);
    }
}

void UVFXManagerComponent::TriggerTensionEffect(EVFXCategory TensionType, float Intensity)
{
    // Find effects of the specified tension category
    for (const FVFXDefinition& EffectDef : VFXDatabase)
    {
        if (EffectDef.Category == TensionType)
        {
            // Modify intensity based on environmental tension
            float ModifiedIntensity = Intensity * (1.0f + CurrentEnvironmentalTension);
            
            // Trigger the effect at the player's location with some randomization
            if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
            {
                FVector EffectLocation = PlayerPawn->GetActorLocation() + 
                    FVector(FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(-500.0f, 500.0f), 0.0f);
                
                PlayVFXEffect(EffectDef.EffectName, EffectLocation);
            }
        }
    }
}

void UVFXManagerComponent::SetEnvironmentalTension(float TensionLevel)
{
    CurrentEnvironmentalTension = FMath::Clamp(TensionLevel, 0.0f, 1.0f);
    
    // Modify all active effects based on new tension level
    for (auto& EffectPair : ActiveEffects)
    {
        if (EffectPair.Value && IsValid(EffectPair.Value))
        {
            FString BaseEffectName = EffectPair.Key.Left(EffectPair.Key.Find(TEXT("_")));
            FVFXDefinition* EffectDef = VFXDatabase.FindByPredicate([&BaseEffectName](const FVFXDefinition& Def)
            {
                return Def.EffectName == BaseEffectName;
            });

            if (EffectDef)
            {
                ModifyEffectForTension(EffectPair.Value, *EffectDef);
            }
        }
    }
}

EVFXLODLevel UVFXManagerComponent::CalculateLODLevel(float DistanceToPlayer)
{
    if (DistanceToPlayer <= 1000.0f)
    {
        return EVFXLODLevel::High;
    }
    else if (DistanceToPlayer <= 3000.0f)
    {
        return EVFXLODLevel::Medium;
    }
    else
    {
        return EVFXLODLevel::Low;
    }
}

UNiagaraSystem* UVFXManagerComponent::GetNiagaraSystemForLOD(const FVFXDefinition& VFXDef, EVFXLODLevel LODLevel)
{
    switch (LODLevel)
    {
        case EVFXLODLevel::High:
            return VFXDef.NiagaraSystem_High.LoadSynchronous();
        case EVFXLODLevel::Medium:
            return VFXDef.NiagaraSystem_Medium.LoadSynchronous();
        case EVFXLODLevel::Low:
            return VFXDef.NiagaraSystem_Low.LoadSynchronous();
        default:
            return VFXDef.NiagaraSystem_Medium.LoadSynchronous();
    }
}

void UVFXManagerComponent::CleanupInactiveEffects()
{
    for (auto It = ActiveEffects.CreateIterator(); It; ++It)
    {
        if (!It->Value || !IsValid(It->Value) || !It->Value->IsActive())
        {
            It.RemoveCurrent();
        }
    }
}

void UVFXManagerComponent::ModifyEffectForTension(UNiagaraComponent* Effect, const FVFXDefinition& EffectDef)
{
    if (!Effect) return;

    // Modify effect parameters based on environmental tension
    float TensionMultiplier = 1.0f + (CurrentEnvironmentalTension * 0.5f);
    
    // Common tension modifications
    Effect->SetFloatParameter(TEXT("TensionMultiplier"), TensionMultiplier);
    Effect->SetFloatParameter(TEXT("EnvironmentalTension"), CurrentEnvironmentalTension);
    
    // Category-specific modifications
    switch (EffectDef.Category)
    {
        case EVFXCategory::DangerIndicators:
            Effect->SetFloatParameter(TEXT("IntensityScale"), TensionMultiplier * 2.0f);
            break;
            
        case EVFXCategory::EnvironmentalAmbient:
            Effect->SetFloatParameter(TEXT("OpacityScale"), 0.5f + (CurrentEnvironmentalTension * 0.5f));
            break;
            
        case EVFXCategory::DinosaurBreathing:
            Effect->SetFloatParameter(TEXT("BreathingRate"), 1.0f + CurrentEnvironmentalTension);
            break;
            
        default:
            break;
    }
}

void UVFXManagerComponent::InitializeDefaultVFXDatabase()
{
    VFXDatabase.Empty();
    
    // Environmental ambient effects
    FVFXDefinition ForestMist;
    ForestMist.EffectName = TEXT("ForestMist");
    ForestMist.Category = EVFXCategory::EnvironmentalAmbient;
    ForestMist.Intensity = EVFXIntensity::Subtle;
    ForestMist.MaxDrawDistance = 8000.0f;
    ForestMist.EmotionalIntent = TEXT("Mysterious, ancient atmosphere");
    VFXDatabase.Add(ForestMist);
    
    // Danger indicators
    FVFXDefinition ThreatPulse;
    ThreatPulse.EffectName = TEXT("ThreatPulse");
    ThreatPulse.Category = EVFXCategory::DangerIndicators;
    ThreatPulse.Intensity = EVFXIntensity::Prominent;
    ThreatPulse.bRequiresPlayerProximity = true;
    ThreatPulse.PlayerProximityRadius = 2000.0f;
    ThreatPulse.EmotionalIntent = TEXT("Imminent danger, heightened alertness");
    VFXDatabase.Add(ThreatPulse);
    
    // Dinosaur breathing
    FVFXDefinition DinoBreath;
    DinoBreath.EffectName = TEXT("DinosaurBreath");
    DinoBreath.Category = EVFXCategory::DinosaurBreathing;
    DinoBreath.Intensity = EVFXIntensity::Moderate;
    DinoBreath.MaxDrawDistance = 5000.0f;
    DinoBreath.EmotionalIntent = TEXT("Living, breathing presence");
    VFXDatabase.Add(DinoBreath);
    
    // Gem effects
    FVFXDefinition GemGlow;
    GemGlow.EffectName = TEXT("TimeGemGlow");
    GemGlow.Category = EVFXCategory::GemEffects;
    GemGlow.Intensity = EVFXIntensity::Dramatic;
    GemGlow.MaxDrawDistance = 10000.0f;
    GemGlow.EmotionalIntent = TEXT("Hope, mystery, temporal power");
    VFXDatabase.Add(GemGlow);
    
    UE_LOG(LogTemp, Log, TEXT("VFX Database initialized with %d default effects"), VFXDatabase.Num());
}