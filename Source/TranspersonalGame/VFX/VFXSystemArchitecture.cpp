#include "VFXSystemArchitecture.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UVFXManagerComponent::UVFXManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms for performance
}

void UVFXManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeEffectDatabase();
}

void UVFXManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Performance management
    CleanupFinishedEffects();
    
    // Cull distant effects based on player location
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        CullDistantEffects(PlayerPawn->GetActorLocation(), EffectCullDistance);
    }
}

void UVFXManagerComponent::PlayEffect(const FString& EffectName, const FVector& Location, const FRotator& Rotation)
{
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX: Maximum active effects reached. Consider increasing limit or optimizing."));
        return;
    }

    const FVFXEffectData* EffectData = EffectDatabase.Find(EffectName);
    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX: Effect '%s' not found in database"), *EffectName);
        return;
    }

    UNiagaraSystem* SystemToUse = GetEffectForLOD(*EffectData);
    if (!SystemToUse)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX: No valid Niagara system for effect '%s' at current LOD"), *EffectName);
        return;
    }

    UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        SystemToUse,
        Location,
        Rotation
    );

    if (NewEffect)
    {
        ActiveEffects.Add(EffectName + FString::Printf(TEXT("_%d"), FMath::RandRange(1000, 9999)), NewEffect);
        
        // Set effect parameters based on category and intensity
        switch (EffectData->Category)
        {
            case EVFXCategory::Creature:
                NewEffect->SetFloatParameter(TEXT("CreatureScale"), 1.0f);
                break;
            case EVFXCategory::Environmental:
                NewEffect->SetFloatParameter(TEXT("WindStrength"), 0.5f);
                break;
            case EVFXCategory::Emotional:
                NewEffect->SetFloatParameter(TEXT("EmotionalIntensity"), static_cast<float>(EffectData->Intensity) / 3.0f);
                break;
        }
    }
}

void UVFXManagerComponent::StopEffect(const FString& EffectName)
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
            break;
        }
    }
}

void UVFXManagerComponent::SetVFXQuality(EVFXLODLevel NewLODLevel)
{
    if (CurrentLODLevel != NewLODLevel)
    {
        CurrentLODLevel = NewLODLevel;
        
        // Restart all active effects with new LOD
        TMap<FString, UNiagaraComponent*> EffectsToRestart = ActiveEffects;
        ActiveEffects.Empty();
        
        for (const auto& Effect : EffectsToRestart)
        {
            if (Effect.Value && IsValid(Effect.Value))
            {
                FVector Location = Effect.Value->GetComponentLocation();
                FRotator Rotation = Effect.Value->GetComponentRotation();
                Effect.Value->DestroyComponent();
                
                // Extract base effect name (remove unique suffix)
                FString BaseEffectName = Effect.Key;
                int32 UnderscoreIndex;
                if (BaseEffectName.FindLastChar('_', UnderscoreIndex))
                {
                    BaseEffectName = BaseEffectName.Left(UnderscoreIndex);
                }
                
                PlayEffect(BaseEffectName, Location, Rotation);
            }
        }
    }
}

void UVFXManagerComponent::PlayCreatureBreathing(AActor* Creature, float IntensityMultiplier)
{
    if (!Creature) return;
    
    FVector BreathLocation = Creature->GetActorLocation() + FVector(0, 0, 50); // Slightly above creature
    PlayEffect(TEXT("CreatureBreathing"), BreathLocation);
    
    // Find the effect and set creature-specific parameters
    for (auto& Effect : ActiveEffects)
    {
        if (Effect.Key.Contains(TEXT("CreatureBreathing")) && Effect.Value)
        {
            Effect.Value->SetFloatParameter(TEXT("BreathIntensity"), IntensityMultiplier);
            Effect.Value->SetVectorParameter(TEXT("CreatureSize"), FVector(1.0f)); // Will be set based on actual creature
            break;
        }
    }
}

void UVFXManagerComponent::PlayFootstepEffect(const FVector& Location, float CreatureSize)
{
    PlayEffect(TEXT("CreatureFootstep"), Location);
    
    // Set size-based parameters
    for (auto& Effect : ActiveEffects)
    {
        if (Effect.Key.Contains(TEXT("CreatureFootstep")) && Effect.Value)
        {
            Effect.Value->SetFloatParameter(TEXT("FootstepScale"), CreatureSize);
            Effect.Value->SetFloatParameter(TEXT("DustAmount"), CreatureSize * 0.5f);
            break;
        }
    }
}

void UVFXManagerComponent::PlayTrustBuildingEffect(AActor* Player, AActor* Creature, float TrustLevel)
{
    if (!Player || !Creature) return;
    
    FVector MidPoint = (Player->GetActorLocation() + Creature->GetActorLocation()) * 0.5f;
    PlayEffect(TEXT("TrustBuilding"), MidPoint);
    
    // Set trust-specific parameters
    for (auto& Effect : ActiveEffects)
    {
        if (Effect.Key.Contains(TEXT("TrustBuilding")) && Effect.Value)
        {
            Effect.Value->SetFloatParameter(TEXT("TrustLevel"), TrustLevel);
            Effect.Value->SetVectorParameter(TEXT("PlayerLocation"), Player->GetActorLocation());
            Effect.Value->SetVectorParameter(TEXT("CreatureLocation"), Creature->GetActorLocation());
            break;
        }
    }
}

void UVFXManagerComponent::SetAtmosphericTension(float TensionLevel)
{
    // Clamp tension level
    TensionLevel = FMath::Clamp(TensionLevel, 0.0f, 1.0f);
    
    // Update all environmental effects
    for (auto& Effect : ActiveEffects)
    {
        if (Effect.Value)
        {
            const FString& EffectName = Effect.Key;
            if (EffectName.Contains(TEXT("Atmospheric")) || EffectName.Contains(TEXT("Ambient")))
            {
                Effect.Value->SetFloatParameter(TEXT("TensionLevel"), TensionLevel);
                Effect.Value->SetFloatParameter(TEXT("ParticleIntensity"), 0.3f + (TensionLevel * 0.7f));
            }
        }
    }
}

void UVFXManagerComponent::PlayAmbientLifeEffect(const FVector& Location, const FString& BiomeType)
{
    FString EffectName = FString::Printf(TEXT("AmbientLife_%s"), *BiomeType);
    PlayEffect(EffectName, Location);
}

void UVFXManagerComponent::CullDistantEffects(const FVector& ViewerLocation, float MaxDistance)
{
    for (auto It = ActiveEffects.CreateIterator(); It; ++It)
    {
        if (It->Value && IsValid(It->Value))
        {
            float Distance = FVector::Dist(ViewerLocation, It->Value->GetComponentLocation());
            if (Distance > MaxDistance)
            {
                It->Value->DestroyComponent();
                It.RemoveCurrent();
            }
        }
    }
}

void UVFXManagerComponent::InitializeEffectDatabase()
{
    // Environmental Effects
    FVFXEffectData AtmosphericTension;
    AtmosphericTension.EffectName = TEXT("AtmosphericTension");
    AtmosphericTension.Category = EVFXCategory::Environmental;
    AtmosphericTension.Intensity = EVFXIntensity::Subtle;
    AtmosphericTension.MaxDistance = 20000.0f;
    EffectDatabase.Add(AtmosphericTension.EffectName, AtmosphericTension);

    // Creature Effects
    FVFXEffectData CreatureBreathing;
    CreatureBreathing.EffectName = TEXT("CreatureBreathing");
    CreatureBreathing.Category = EVFXCategory::Creature;
    CreatureBreathing.Intensity = EVFXIntensity::Subtle;
    CreatureBreathing.MaxDistance = 1000.0f;
    EffectDatabase.Add(CreatureBreathing.EffectName, CreatureBreathing);

    FVFXEffectData CreatureFootstep;
    CreatureFootstep.EffectName = TEXT("CreatureFootstep");
    CreatureFootstep.Category = EVFXCategory::Creature;
    CreatureFootstep.Intensity = EVFXIntensity::Noticeable;
    CreatureFootstep.MaxDistance = 2000.0f;
    EffectDatabase.Add(CreatureFootstep.EffectName, CreatureFootstep);

    // Emotional Effects
    FVFXEffectData TrustBuilding;
    TrustBuilding.EffectName = TEXT("TrustBuilding");
    TrustBuilding.Category = EVFXCategory::Emotional;
    TrustBuilding.Intensity = EVFXIntensity::Prominent;
    TrustBuilding.MaxDistance = 500.0f;
    TrustBuilding.bAffectsGameplay = true;
    EffectDatabase.Add(TrustBuilding.EffectName, TrustBuilding);

    UE_LOG(LogTemp, Log, TEXT("VFX: Effect database initialized with %d effects"), EffectDatabase.Num());
}

UNiagaraSystem* UVFXManagerComponent::GetEffectForLOD(const FVFXEffectData& EffectData) const
{
    switch (CurrentLODLevel)
    {
        case EVFXLODLevel::High:
            return EffectData.NiagaraSystem_High.LoadSynchronous();
        case EVFXLODLevel::Medium:
            return EffectData.NiagaraSystem_Medium.LoadSynchronous();
        case EVFXLODLevel::Low:
            return EffectData.NiagaraSystem_Low.LoadSynchronous();
        default:
            return EffectData.NiagaraSystem_Medium.LoadSynchronous();
    }
}

void UVFXManagerComponent::CleanupFinishedEffects()
{
    for (auto It = ActiveEffects.CreateIterator(); It; ++It)
    {
        if (!It->Value || !IsValid(It->Value) || !It->Value->IsActive())
        {
            if (It->Value && IsValid(It->Value))
            {
                It->Value->DestroyComponent();
            }
            It.RemoveCurrent();
        }
    }
}