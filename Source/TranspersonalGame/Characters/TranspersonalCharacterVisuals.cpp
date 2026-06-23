#include "TranspersonalCharacterVisuals.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Character.h"

UTranspersonalCharacterVisuals::UTranspersonalCharacterVisuals()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second — not every frame
}

void UTranspersonalCharacterVisuals::BeginPlay()
{
    Super::BeginPlay();

    // Grab skeletal mesh from owner character
    if (AActor* Owner = GetOwner())
    {
        CharacterMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (CharacterMesh)
        {
            RefreshMaterialParameters();
        }
    }
}

void UTranspersonalCharacterVisuals::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Passive dirt accumulation while moving in jungle
    DirtAccumulationTimer += DeltaTime;
    if (DirtAccumulationTimer >= 1.0f)
    {
        AccumulateDirt(DirtAccumulationRate);
        DirtAccumulationTimer = 0.0f;
    }
}

void UTranspersonalCharacterVisuals::AccumulateDirt(float Amount)
{
    AppearanceData.DirtLevel = FMath::Clamp(AppearanceData.DirtLevel + Amount, 0.0f, 1.0f);

    // Update material only when dirt crosses visible threshold
    if (CharacterMesh && FMath::Fmod(AppearanceData.DirtLevel, 0.1f) < Amount)
    {
        RefreshMaterialParameters();
    }
}

void UTranspersonalCharacterVisuals::ApplyWound(float Severity)
{
    AppearanceData.WoundLevel = FMath::Clamp(AppearanceData.WoundLevel + Severity, 0.0f, 1.0f);

    // Severe wounds change skin condition
    if (AppearanceData.WoundLevel > 0.6f)
    {
        AppearanceData.SkinCondition = EChar_SkinCondition::Wounded;
    }

    RefreshMaterialParameters();
}

void UTranspersonalCharacterVisuals::UpdateNutritionMorphs(float HungerLevel)
{
    // HungerLevel: 0=full, 1=starving — invert for nutrition
    AppearanceData.NutritionLevel = FMath::Clamp(1.0f - HungerLevel, 0.0f, 1.0f);

    if (AppearanceData.NutritionLevel < 0.2f)
    {
        AppearanceData.SkinCondition = EChar_SkinCondition::Starving;
    }

    // Drive morph targets on skeletal mesh
    if (CharacterMesh)
    {
        // Starvation morph: sunken cheeks, visible ribs
        CharacterMesh->SetMorphTarget(FName("Starvation"), 1.0f - AppearanceData.NutritionLevel);
        // Muscle definition: more defined when well-fed
        CharacterMesh->SetMorphTarget(FName("MuscleDefinition"), AppearanceData.NutritionLevel * 0.7f);
    }
}

void UTranspersonalCharacterVisuals::AddScar()
{
    AppearanceData.ScarCount = FMath::Clamp(AppearanceData.ScarCount + 1, 0, 10);
    RefreshMaterialParameters();

    UE_LOG(LogTemp, Log, TEXT("[CharacterVisuals] Scar added. Total scars: %d"), AppearanceData.ScarCount);
}

void UTranspersonalCharacterVisuals::ResetAppearance()
{
    AppearanceData.DirtLevel = 0.0f;
    AppearanceData.WoundLevel = 0.0f;
    AppearanceData.NutritionLevel = 0.5f;
    AppearanceData.SkinCondition = EChar_SkinCondition::Healthy;
    RefreshMaterialParameters();
}

void UTranspersonalCharacterVisuals::SetClothingType(EChar_ClothingType NewType)
{
    AppearanceData.ClothingType = NewType;
    // In full implementation: swap clothing mesh section or material slot
    UE_LOG(LogTemp, Log, TEXT("[CharacterVisuals] Clothing changed to: %d"), (int32)NewType);
}

void UTranspersonalCharacterVisuals::RefreshMaterialParameters()
{
    if (!CharacterMesh)
    {
        return;
    }

    // Create dynamic material instance if not already done
    UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(CharacterMesh->GetMaterial(0));
    if (!DynMat)
    {
        UMaterialInterface* BaseMat = CharacterMesh->GetMaterial(0);
        if (BaseMat)
        {
            DynMat = CharacterMesh->CreateAndSetMaterialInstanceDynamic(0);
        }
    }

    if (DynMat)
    {
        // Drive material parameters
        DynMat->SetScalarParameterValue(FName("DirtLevel"), AppearanceData.DirtLevel);
        DynMat->SetScalarParameterValue(FName("WoundLevel"), AppearanceData.WoundLevel);
        DynMat->SetScalarParameterValue(FName("NutritionLevel"), AppearanceData.NutritionLevel);
        DynMat->SetScalarParameterValue(FName("ScarCount"), (float)AppearanceData.ScarCount);

        // Skin condition tint
        FLinearColor SkinTint = FLinearColor::White;
        switch (AppearanceData.SkinCondition)
        {
            case EChar_SkinCondition::Sunburned:
                SkinTint = FLinearColor(1.0f, 0.75f, 0.65f, 1.0f);
                break;
            case EChar_SkinCondition::Wounded:
                SkinTint = FLinearColor(0.85f, 0.7f, 0.7f, 1.0f);
                break;
            case EChar_SkinCondition::Exhausted:
                SkinTint = FLinearColor(0.8f, 0.8f, 0.78f, 1.0f);
                break;
            case EChar_SkinCondition::Starving:
                SkinTint = FLinearColor(0.7f, 0.68f, 0.65f, 1.0f);
                break;
            default:
                SkinTint = FLinearColor::White;
                break;
        }
        DynMat->SetVectorParameterValue(FName("SkinConditionTint"), SkinTint);
    }
}
