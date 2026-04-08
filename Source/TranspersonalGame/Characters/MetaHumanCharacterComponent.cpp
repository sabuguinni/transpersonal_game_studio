#include "MetaHumanCharacterComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimBlueprint.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

UMetaHumanCharacterComponent::UMetaHumanCharacterComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    // Initialize default values
    CurrentSurvivalState = ESurvivalCondition::Fresh;
    DaysSurvived = 0;
    HealthCondition = 1.0f;
    MentalState = 1.0f;
    
    // Set default visual traits
    CurrentVisualTraits.Height = 1.0f;
    CurrentVisualTraits.Weight = 1.0f;
    CurrentVisualTraits.Muscle = 0.5f;
    CurrentVisualTraits.FaceWidth = 0.5f;
    CurrentVisualTraits.JawWidth = 0.5f;
    CurrentVisualTraits.EyeSize = 0.5f;
    CurrentVisualTraits.NoseSize = 0.5f;
    CurrentVisualTraits.SkinDamage = 0.0f;
    CurrentVisualTraits.Fatigue = 0.0f;
    CurrentVisualTraits.Weathering = 0.0f;
}

void UMetaHumanCharacterComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the skeletal mesh component on the owner
    if (AActor* Owner = GetOwner())
    {
        CachedMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
    }
    
    // Apply the initial archetype if set
    if (CharacterArchetype.IsValid())
    {
        if (UCharacterArchetype* LoadedArchetype = CharacterArchetype.LoadSynchronous())
        {
            ApplyArchetype(LoadedArchetype);
        }
    }
    
    SetupMetaHumanMesh();
}

void UMetaHumanCharacterComponent::TickComponent(float DeltaTime, ELevelTick TickType, 
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update material parameters based on current state
    UpdateMaterialParameters();
}

void UMetaHumanCharacterComponent::ApplyArchetype(UCharacterArchetype* NewArchetype)
{
    if (!NewArchetype)
    {
        UE_LOG(LogTemp, Warning, TEXT("MetaHumanCharacterComponent: Attempted to apply null archetype"));
        return;
    }
    
    CharacterArchetype = NewArchetype;
    
    // Copy archetype data to component
    CurrentVisualTraits = NewArchetype->VisualTraits;
    CurrentClothing = NewArchetype->ClothingSetup;
    CurrentSurvivalState = NewArchetype->SurvivalState;
    DaysSurvived = NewArchetype->DaysInJurassic;
    
    // Apply the MetaHuman mesh
    if (NewArchetype->MetaHumanMesh.IsValid())
    {
        if (CachedMeshComponent)
        {
            USkeletalMesh* LoadedMesh = NewArchetype->MetaHumanMesh.LoadSynchronous();
            CachedMeshComponent->SetSkeletalMesh(LoadedMesh);
        }
    }
    
    // Apply animation blueprint
    if (NewArchetype->CharacterAnimBP.IsValid())
    {
        if (CachedMeshComponent)
        {
            UClass* AnimBPClass = NewArchetype->CharacterAnimBP.LoadSynchronous();
            CachedMeshComponent->SetAnimInstanceClass(AnimBPClass);
        }
    }
    
    RefreshCharacterMesh();
    
    UE_LOG(LogTemp, Log, TEXT("Applied archetype: %s"), *NewArchetype->CharacterName);
}

void UMetaHumanCharacterComponent::UpdateVisualTraits(const FCharacterVisualTraits& NewTraits)
{
    CurrentVisualTraits = NewTraits;
    ApplyBodyMorphs();
    ApplyFacialMorphs();
    
    OnAppearanceChanged.Broadcast(CurrentVisualTraits);
}

void UMetaHumanCharacterComponent::UpdateClothing(const FCharacterClothing& NewClothing)
{
    CurrentClothing = NewClothing;
    ApplyClothingMeshes();
    UpdateClothingCondition();
}

void UMetaHumanCharacterComponent::AdvanceSurvivalState(int32 DaysElapsed)
{
    DaysSurvived += DaysElapsed;
    
    // Update survival state based on days survived
    if (DaysSurvived < 30)
    {
        CurrentSurvivalState = ESurvivalCondition::Fresh;
    }
    else if (DaysSurvived < 180)
    {
        CurrentSurvivalState = ESurvivalCondition::Weathered;
    }
    else if (DaysSurvived < 365)
    {
        CurrentSurvivalState = ESurvivalCondition::Hardened;
    }
    else
    {
        CurrentSurvivalState = ESurvivalCondition::Broken;
    }
    
    CalculateSurvivalEffects();
    ApplySurvivalWear();
}

void UMetaHumanCharacterComponent::ApplyInjury(float InjurySeverity)
{
    HealthCondition = FMath::Clamp(HealthCondition - InjurySeverity, 0.0f, 1.0f);
    
    // Increase skin damage based on injury
    CurrentVisualTraits.SkinDamage = FMath::Clamp(
        CurrentVisualTraits.SkinDamage + InjurySeverity * 0.3f, 0.0f, 1.0f);
    
    UpdateVisualTraits(CurrentVisualTraits);
}

void UMetaHumanCharacterComponent::ApplyPsychologicalStress(float StressLevel)
{
    MentalState = FMath::Clamp(MentalState - StressLevel, 0.0f, 1.0f);
    
    // Increase fatigue based on stress
    CurrentVisualTraits.Fatigue = FMath::Clamp(
        CurrentVisualTraits.Fatigue + StressLevel * 0.5f, 0.0f, 1.0f);
    
    UpdateVisualTraits(CurrentVisualTraits);
}

void UMetaHumanCharacterComponent::RefreshCharacterMesh()
{
    if (!CachedMeshComponent)
        return;
    
    ApplyBodyMorphs();
    ApplyFacialMorphs();
    ApplyClothingMeshes();
    ApplySurvivalWear();
    UpdateMaterialParameters();
}

void UMetaHumanCharacterComponent::ApplySurvivalWear()
{
    // Calculate weathering based on survival state
    float WeatheringAmount = 0.0f;
    
    switch (CurrentSurvivalState)
    {
        case ESurvivalCondition::Fresh:
            WeatheringAmount = 0.1f;
            break;
        case ESurvivalCondition::Weathered:
            WeatheringAmount = 0.4f;
            break;
        case ESurvivalCondition::Hardened:
            WeatheringAmount = 0.7f;
            break;
        case ESurvivalCondition::Broken:
            WeatheringAmount = 1.0f;
            break;
    }
    
    CurrentVisualTraits.Weathering = WeatheringAmount;
    CurrentClothing.ClothingWear = WeatheringAmount * 0.8f;
    CurrentClothing.DirtLevel = WeatheringAmount * 0.9f;
    
    UpdateVisualTraits(CurrentVisualTraits);
    UpdateClothing(CurrentClothing);
}

void UMetaHumanCharacterComponent::UpdateClothingCondition()
{
    // This will be expanded when clothing mesh system is implemented
    // For now, just update material parameters
    UpdateMaterialParameters();
}

FString UMetaHumanCharacterComponent::GetCharacterDisplayName() const
{
    if (CharacterArchetype.IsValid())
    {
        if (UCharacterArchetype* LoadedArchetype = CharacterArchetype.LoadSynchronous())
        {
            return LoadedArchetype->CharacterName;
        }
    }
    
    return TEXT("Unknown Survivor");
}

FText UMetaHumanCharacterComponent::GetCharacterDescription() const
{
    if (CharacterArchetype.IsValid())
    {
        if (UCharacterArchetype* LoadedArchetype = CharacterArchetype.LoadSynchronous())
        {
            return LoadedArchetype->CharacterDescription;
        }
    }
    
    return FText::FromString(TEXT("A survivor in the Jurassic world"));
}

float UMetaHumanCharacterComponent::GetOverallCondition() const
{
    return (HealthCondition + MentalState) * 0.5f;
}

void UMetaHumanCharacterComponent::CalculateSurvivalEffects()
{
    // Calculate effects based on survival time and conditions
    float SurvivalRatio = FMath::Clamp(DaysSurvived / 365.0f, 0.0f, 1.0f);
    
    // Gradual deterioration over time
    CurrentVisualTraits.Fatigue = FMath::Clamp(SurvivalRatio * 0.6f, 0.0f, 1.0f);
    CurrentVisualTraits.Weathering = FMath::Clamp(SurvivalRatio * 0.8f, 0.0f, 1.0f);
    
    // Health affects appearance
    CurrentVisualTraits.SkinDamage = FMath::Clamp((1.0f - HealthCondition) * 0.7f, 0.0f, 1.0f);
}

void UMetaHumanCharacterComponent::UpdateMaterialParameters()
{
    if (!CachedMeshComponent)
        return;
    
    // Update material parameters for survival effects
    for (int32 MaterialIndex = 0; MaterialIndex < CachedMeshComponent->GetNumMaterials(); MaterialIndex++)
    {
        if (UMaterialInstanceDynamic* DynMaterial = CachedMeshComponent->CreateDynamicMaterialInstance(MaterialIndex))
        {
            // Apply survival wear parameters
            DynMaterial->SetScalarParameterValue(TEXT("SkinDamage"), CurrentVisualTraits.SkinDamage);
            DynMaterial->SetScalarParameterValue(TEXT("Fatigue"), CurrentVisualTraits.Fatigue);
            DynMaterial->SetScalarParameterValue(TEXT("Weathering"), CurrentVisualTraits.Weathering);
            DynMaterial->SetScalarParameterValue(TEXT("HealthCondition"), HealthCondition);
            DynMaterial->SetScalarParameterValue(TEXT("MentalState"), MentalState);
            
            // Clothing wear parameters
            DynMaterial->SetScalarParameterValue(TEXT("ClothingWear"), CurrentClothing.ClothingWear);
            DynMaterial->SetScalarParameterValue(TEXT("DirtLevel"), CurrentClothing.DirtLevel);
            DynMaterial->SetScalarParameterValue(TEXT("BloodStains"), CurrentClothing.BloodStains);
        }
    }
}

void UMetaHumanCharacterComponent::SetupMetaHumanMesh()
{
    if (!CachedMeshComponent)
        return;
    
    // Apply appropriate base mesh based on archetype gender
    if (CharacterArchetype.IsValid())
    {
        if (UCharacterArchetype* LoadedArchetype = CharacterArchetype.LoadSynchronous())
        {
            TSoftObjectPtr<USkeletalMesh> TargetMesh = LoadedArchetype->bIsMale ? BaseMaleMesh : BaseFemaleMesh;
            
            if (TargetMesh.IsValid())
            {
                USkeletalMesh* LoadedMesh = TargetMesh.LoadSynchronous();
                CachedMeshComponent->SetSkeletalMesh(LoadedMesh);
            }
        }
    }
    
    // Apply animation blueprint
    if (CharacterAnimBlueprint.IsValid())
    {
        UClass* AnimBPClass = CharacterAnimBlueprint.LoadSynchronous();
        CachedMeshComponent->SetAnimInstanceClass(AnimBPClass);
    }
}

void UMetaHumanCharacterComponent::ApplyBodyMorphs()
{
    if (!CachedMeshComponent)
        return;
    
    // Apply body morphs based on visual traits
    // These correspond to MetaHuman morph target names
    CachedMeshComponent->SetMorphTarget(TEXT("Height"), CurrentVisualTraits.Height);
    CachedMeshComponent->SetMorphTarget(TEXT("Weight"), CurrentVisualTraits.Weight);
    CachedMeshComponent->SetMorphTarget(TEXT("Muscle"), CurrentVisualTraits.Muscle);
}

void UMetaHumanCharacterComponent::ApplyFacialMorphs()
{
    if (!CachedMeshComponent)
        return;
    
    // Apply facial morphs based on visual traits
    CachedMeshComponent->SetMorphTarget(TEXT("FaceWidth"), CurrentVisualTraits.FaceWidth);
    CachedMeshComponent->SetMorphTarget(TEXT("JawWidth"), CurrentVisualTraits.JawWidth);
    CachedMeshComponent->SetMorphTarget(TEXT("EyeSize"), CurrentVisualTraits.EyeSize);
    CachedMeshComponent->SetMorphTarget(TEXT("NoseSize"), CurrentVisualTraits.NoseSize);
}

void UMetaHumanCharacterComponent::ApplyClothingMeshes()
{
    // This will be expanded when modular clothing system is implemented
    // For now, clothing is handled through material parameters
    UpdateMaterialParameters();
}