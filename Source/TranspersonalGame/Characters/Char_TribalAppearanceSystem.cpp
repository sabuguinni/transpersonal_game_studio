#include "Char_TribalAppearanceSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

UChar_TribalAppearanceSystem::UChar_TribalAppearanceSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default tribal appearance
    CurrentAppearance = FChar_TribalAppearance();
    
    // Initialize material arrays (will be populated in BeginPlay)
    SkinToneMaterials.SetNum(5); // Number of skin tone options
    ClothingMaterials.SetNum(5); // Number of clothing options
    TribalMarkingMaterials.SetNum(6); // Number of marking options
}

void UChar_TribalAppearanceSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Load default materials if not set in Blueprint
    if (SkinToneMaterials.Num() == 0 || !SkinToneMaterials[0].IsValid())
    {
        // Try to load basic materials from Engine
        TSoftObjectPtr<UMaterialInterface> BaseMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Engine/BasicShapes/BasicShapeMaterial")));
        
        for (int32 i = 0; i < 5; i++)
        {
            SkinToneMaterials.Add(BaseMaterial);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("TribalAppearanceSystem initialized for %s"), *GetOwner()->GetName());
}

void UChar_TribalAppearanceSystem::ApplyTribalAppearance(USkeletalMeshComponent* CharacterMesh)
{
    if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalAppearanceSystem: No character mesh provided"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Applying tribal appearance to %s"), *CharacterMesh->GetOwner()->GetName());

    // Apply skin tone material
    ApplySkinToneMaterial(CharacterMesh);
    
    // Apply tribal markings
    ApplyTribalMarkingMaterial(CharacterMesh);
    
    // Apply clothing materials
    ApplyClothingMaterial(CharacterMesh);
    
    // Apply body build scaling
    ApplyBodyBuildScale(CharacterMesh);

    UE_LOG(LogTemp, Log, TEXT("Tribal appearance applied: %s"), *GetAppearanceDescription());
}

void UChar_TribalAppearanceSystem::RandomizeTribalAppearance()
{
    // Randomize skin tone
    int32 SkinToneIndex = FMath::RandRange(0, 4);
    CurrentAppearance.SkinTone = static_cast<EChar_SkinTone>(SkinToneIndex);
    
    // Randomize body build
    int32 BodyBuildIndex = FMath::RandRange(0, 4);
    CurrentAppearance.BodyBuild = static_cast<EChar_BodyBuild>(BodyBuildIndex);
    
    // Randomize tribal markings
    int32 MarkingsIndex = FMath::RandRange(0, 5);
    CurrentAppearance.TribalMarkings = static_cast<EChar_TribalMarkings>(MarkingsIndex);
    
    // Randomize clothing
    int32 ClothingIndex = FMath::RandRange(0, 4);
    CurrentAppearance.Clothing = static_cast<EChar_TribalClothing>(ClothingIndex);
    
    // Randomize hair color (various browns and blacks)
    float HairHue = FMath::RandRange(0.02f, 0.15f); // Brown to black range
    CurrentAppearance.HairColor = FLinearColor::MakeFromHSV8(
        HairHue * 255, 
        FMath::RandRange(100, 200), 
        FMath::RandRange(20, 80)
    );
    
    // Randomize eye color (browns, greens, blues)
    float EyeHue = FMath::RandRange(0.0f, 0.7f);
    CurrentAppearance.EyeColor = FLinearColor::MakeFromHSV8(
        EyeHue * 255,
        FMath::RandRange(80, 180),
        FMath::RandRange(40, 120)
    );
    
    // Randomize weathering and muscle definition
    CurrentAppearance.WeatheringIntensity = FMath::RandRange(0.2f, 0.8f);
    CurrentAppearance.MuscleDefinition = FMath::RandRange(0.3f, 0.9f);
    
    UE_LOG(LogTemp, Log, TEXT("Randomized tribal appearance: %s"), *GetAppearanceDescription());
}

void UChar_TribalAppearanceSystem::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    CurrentAppearance.SkinTone = NewSkinTone;
    UE_LOG(LogTemp, Log, TEXT("Set skin tone to: %d"), static_cast<int32>(NewSkinTone));
}

void UChar_TribalAppearanceSystem::SetBodyBuild(EChar_BodyBuild NewBodyBuild)
{
    CurrentAppearance.BodyBuild = NewBodyBuild;
    UE_LOG(LogTemp, Log, TEXT("Set body build to: %d"), static_cast<int32>(NewBodyBuild));
}

void UChar_TribalAppearanceSystem::SetTribalMarkings(EChar_TribalMarkings NewMarkings)
{
    CurrentAppearance.TribalMarkings = NewMarkings;
    UE_LOG(LogTemp, Log, TEXT("Set tribal markings to: %d"), static_cast<int32>(NewMarkings));
}

void UChar_TribalAppearanceSystem::SetTribalClothing(EChar_TribalClothing NewClothing)
{
    CurrentAppearance.Clothing = NewClothing;
    UE_LOG(LogTemp, Log, TEXT("Set tribal clothing to: %d"), static_cast<int32>(NewClothing));
}

FString UChar_TribalAppearanceSystem::GetAppearanceDescription() const
{
    FString Description = FString::Printf(
        TEXT("SkinTone:%d, BodyBuild:%d, Markings:%d, Clothing:%d, Weathering:%.2f, Muscle:%.2f"),
        static_cast<int32>(CurrentAppearance.SkinTone),
        static_cast<int32>(CurrentAppearance.BodyBuild),
        static_cast<int32>(CurrentAppearance.TribalMarkings),
        static_cast<int32>(CurrentAppearance.Clothing),
        CurrentAppearance.WeatheringIntensity,
        CurrentAppearance.MuscleDefinition
    );
    
    return Description;
}

void UChar_TribalAppearanceSystem::ApplySkinToneMaterial(USkeletalMeshComponent* Mesh)
{
    if (!Mesh) return;
    
    int32 SkinToneIndex = static_cast<int32>(CurrentAppearance.SkinTone);
    
    // Check if we have a cached material
    if (LoadedSkinMaterials.Contains(CurrentAppearance.SkinTone))
    {
        UMaterialInterface* CachedMaterial = LoadedSkinMaterials[CurrentAppearance.SkinTone];
        if (CachedMaterial)
        {
            Mesh->SetMaterial(0, CachedMaterial);
            return;
        }
    }
    
    // Try to load material from array
    if (SkinToneMaterials.IsValidIndex(SkinToneIndex) && SkinToneMaterials[SkinToneIndex].IsValid())
    {
        UMaterialInterface* SkinMaterial = SkinToneMaterials[SkinToneIndex].LoadSynchronous();
        if (SkinMaterial)
        {
            Mesh->SetMaterial(0, SkinMaterial);
            LoadedSkinMaterials.Add(CurrentAppearance.SkinTone, SkinMaterial);
            UE_LOG(LogTemp, Log, TEXT("Applied skin tone material: %s"), *SkinMaterial->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Skin tone material not found for index: %d"), SkinToneIndex);
    }
}

void UChar_TribalAppearanceSystem::ApplyTribalMarkingMaterial(USkeletalMeshComponent* Mesh)
{
    if (!Mesh) return;
    
    // Skip if no markings
    if (CurrentAppearance.TribalMarkings == EChar_TribalMarkings::None)
    {
        return;
    }
    
    int32 MarkingIndex = static_cast<int32>(CurrentAppearance.TribalMarkings);
    
    // Check cached materials
    if (LoadedMarkingMaterials.Contains(CurrentAppearance.TribalMarkings))
    {
        UMaterialInterface* CachedMaterial = LoadedMarkingMaterials[CurrentAppearance.TribalMarkings];
        if (CachedMaterial)
        {
            // Apply to material slot 1 (overlay)
            if (Mesh->GetNumMaterials() > 1)
            {
                Mesh->SetMaterial(1, CachedMaterial);
            }
            return;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied tribal markings: %d"), MarkingIndex);
}

void UChar_TribalAppearanceSystem::ApplyClothingMaterial(USkeletalMeshComponent* Mesh)
{
    if (!Mesh) return;
    
    int32 ClothingIndex = static_cast<int32>(CurrentAppearance.Clothing);
    
    // Check cached materials
    if (LoadedClothingMaterials.Contains(CurrentAppearance.Clothing))
    {
        UMaterialInterface* CachedMaterial = LoadedClothingMaterials[CurrentAppearance.Clothing];
        if (CachedMaterial)
        {
            // Apply to material slot 2 (clothing)
            if (Mesh->GetNumMaterials() > 2)
            {
                Mesh->SetMaterial(2, CachedMaterial);
            }
            return;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied tribal clothing: %d"), ClothingIndex);
}

void UChar_TribalAppearanceSystem::ApplyBodyBuildScale(USkeletalMeshComponent* Mesh)
{
    if (!Mesh) return;
    
    FVector BaseScale = FVector(1.0f, 1.0f, 1.0f);
    
    // Adjust scale based on body build
    switch (CurrentAppearance.BodyBuild)
    {
        case EChar_BodyBuild::Lean:
            BaseScale = FVector(0.9f, 0.9f, 1.05f); // Thinner, slightly taller
            break;
        case EChar_BodyBuild::Muscular:
            BaseScale = FVector(1.1f, 1.1f, 1.0f); // Broader, more muscular
            break;
        case EChar_BodyBuild::Stocky:
            BaseScale = FVector(1.15f, 1.15f, 0.95f); // Wider, shorter
            break;
        case EChar_BodyBuild::Tall:
            BaseScale = FVector(0.95f, 0.95f, 1.15f); // Thinner, taller
            break;
        case EChar_BodyBuild::Elder:
            BaseScale = FVector(0.9f, 0.9f, 0.95f); // Smaller, slightly hunched
            break;
    }
    
    // Apply muscle definition scaling
    float MuscleScale = 1.0f + (CurrentAppearance.MuscleDefinition * 0.2f);
    BaseScale.X *= MuscleScale;
    BaseScale.Y *= MuscleScale;
    
    Mesh->SetWorldScale3D(BaseScale);
    
    UE_LOG(LogTemp, Log, TEXT("Applied body build scale: %s"), *BaseScale.ToString());
}