#include "Char_TribalCharacterManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

AChar_TribalCharacterManager::AChar_TribalCharacterManager()
{
    PrimaryActorTick.bCanEverTick = false;
    MaxCharactersToGenerate = 50;

    // Set default root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AChar_TribalCharacterManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("TribalCharacterManager initialized"));
}

void AChar_TribalCharacterManager::GenerateTribalCharacter(const FTransform& SpawnTransform, bool bIsMale, EChar_TribalRole Role)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world for character generation"));
        return;
    }

    if (GeneratedCharacters.Num() >= MaxCharactersToGenerate)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum character limit reached: %d"), MaxCharactersToGenerate);
        return;
    }

    // Load base character class
    UClass* CharacterClass = LoadClass<ACharacter>(nullptr, TEXT("/Script/Engine.Character"));
    if (!CharacterClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Character class"));
        return;
    }

    // Spawn the character
    AActor* NewCharacter = GetWorld()->SpawnActor<AActor>(CharacterClass, SpawnTransform);
    if (!NewCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn character"));
        return;
    }

    // Generate and apply appearance
    FChar_TribalAppearance Appearance = GenerateRandomAppearance(bIsMale, Role);
    ApplyAppearanceToCharacter(NewCharacter, Appearance);

    // Generate and apply clothing
    FChar_TribalClothing Clothing = GenerateRandomClothing(Role);
    ApplyClothingToCharacter(NewCharacter, Clothing);

    // Set character label based on role and gender
    FString CharacterLabel = FString::Printf(TEXT("Tribal_%s_%s"), 
        bIsMale ? TEXT("Male") : TEXT("Female"),
        *UEnum::GetValueAsString(Role));
    NewCharacter->SetActorLabel(CharacterLabel);

    GeneratedCharacters.Add(NewCharacter);
    UE_LOG(LogTemp, Log, TEXT("Generated tribal character: %s"), *CharacterLabel);
}

void AChar_TribalCharacterManager::GenerateRandomTribalGroup(int32 GroupSize, const FVector& CenterLocation, float SpreadRadius)
{
    for (int32 i = 0; i < GroupSize && GeneratedCharacters.Num() < MaxCharactersToGenerate; i++)
    {
        // Random position within spread radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpreadRadius, SpreadRadius),
            FMath::RandRange(-SpreadRadius, SpreadRadius),
            0.0f
        );
        FVector SpawnLocation = CenterLocation + RandomOffset;

        // Random gender and role
        bool bIsMale = FMath::RandBool();
        EChar_TribalRole Role = static_cast<EChar_TribalRole>(FMath::RandRange(0, 5));

        FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation, FVector::OneVector);
        GenerateTribalCharacter(SpawnTransform, bIsMale, Role);
    }
}

FChar_TribalAppearance AChar_TribalCharacterManager::GenerateRandomAppearance(bool bIsMale, EChar_TribalRole Role)
{
    FChar_TribalAppearance Appearance;

    // Skin tone based on role and randomization
    Appearance.SkinTone = GetSkinToneForRole(Role);
    
    // Add random variation to skin tone
    float SkinVariation = FMath::RandRange(-0.1f, 0.1f);
    Appearance.SkinTone.R = FMath::Clamp(Appearance.SkinTone.R + SkinVariation, 0.3f, 0.9f);
    Appearance.SkinTone.G = FMath::Clamp(Appearance.SkinTone.G + SkinVariation, 0.2f, 0.8f);
    Appearance.SkinTone.B = FMath::Clamp(Appearance.SkinTone.B + SkinVariation, 0.1f, 0.6f);

    // Hair color variations
    TArray<FLinearColor> HairColors = {
        FLinearColor(0.1f, 0.05f, 0.02f, 1.0f),  // Dark brown
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f),   // Brown
        FLinearColor(0.05f, 0.05f, 0.05f, 1.0f), // Black
        FLinearColor(0.3f, 0.2f, 0.1f, 1.0f)     // Light brown
    };
    Appearance.HairColor = HairColors[FMath::RandRange(0, HairColors.Num() - 1)];

    // Muscle mass based on role and gender
    Appearance.MuscleMass = GetMuscleMassForRole(Role, bIsMale);
    
    // Body fat (lower for hunters/warriors, higher for elders)
    switch (Role)
    {
        case EChar_TribalRole::Hunter:
        case EChar_TribalRole::Warrior:
            Appearance.BodyFat = FMath::RandRange(0.08f, 0.15f);
            break;
        case EChar_TribalRole::Elder:
            Appearance.BodyFat = FMath::RandRange(0.2f, 0.35f);
            break;
        default:
            Appearance.BodyFat = FMath::RandRange(0.12f, 0.25f);
            break;
    }

    // Height variation
    float BaseHeight = bIsMale ? 1.0f : 0.92f;
    Appearance.Height = BaseHeight + FMath::RandRange(-0.08f, 0.08f);

    return Appearance;
}

FChar_TribalClothing AChar_TribalCharacterManager::GenerateRandomClothing(EChar_TribalRole Role)
{
    FChar_TribalClothing Clothing;

    // Clothing color based on role
    Clothing.ClothingColor = GetClothingColorForRole(Role);
    
    // Add random variation
    float ColorVariation = FMath::RandRange(-0.1f, 0.1f);
    Clothing.ClothingColor.R = FMath::Clamp(Clothing.ClothingColor.R + ColorVariation, 0.1f, 0.8f);
    Clothing.ClothingColor.G = FMath::Clamp(Clothing.ClothingColor.G + ColorVariation, 0.1f, 0.7f);
    Clothing.ClothingColor.B = FMath::Clamp(Clothing.ClothingColor.B + ColorVariation, 0.1f, 0.5f);

    return Clothing;
}

void AChar_TribalCharacterManager::ApplyAppearanceToCharacter(AActor* Character, const FChar_TribalAppearance& Appearance)
{
    if (!Character)
        return;

    ACharacter* CharacterPawn = Cast<ACharacter>(Character);
    if (!CharacterPawn)
        return;

    USkeletalMeshComponent* MeshComp = CharacterPawn->GetMesh();
    if (!MeshComp)
        return;

    // Try to load a basic skeletal mesh if none is set
    if (!MeshComp->GetSkeletalMeshAsset())
    {
        USkeletalMesh* DefaultMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
        if (DefaultMesh)
        {
            MeshComp->SetSkeletalMesh(DefaultMesh);
        }
    }

    // Create dynamic material instance for skin
    if (MeshComp->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = MeshComp->CreateDynamicMaterialInstance(0);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("SkinColor"), Appearance.SkinTone);
            DynamicMaterial->SetVectorParameterValue(TEXT("HairColor"), Appearance.HairColor);
            DynamicMaterial->SetScalarParameterValue(TEXT("MuscleMass"), Appearance.MuscleMass);
        }
    }

    // Apply height scaling
    Character->SetActorScale3D(FVector(1.0f, 1.0f, Appearance.Height));
}

void AChar_TribalCharacterManager::ApplyClothingToCharacter(AActor* Character, const FChar_TribalClothing& Clothing)
{
    if (!Character)
        return;

    // For now, just log clothing application
    // In a full implementation, this would attach clothing meshes as components
    UE_LOG(LogTemp, Log, TEXT("Applied clothing to character: %s"), *Character->GetName());
}

void AChar_TribalCharacterManager::GenerateTestCharacters()
{
    if (!GetWorld())
        return;

    ClearGeneratedCharacters();

    FVector BaseLocation = GetActorLocation();
    GenerateRandomTribalGroup(10, BaseLocation, 1000.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Generated %d test characters"), GeneratedCharacters.Num());
}

void AChar_TribalCharacterManager::ClearGeneratedCharacters()
{
    for (AActor* Character : GeneratedCharacters)
    {
        if (IsValid(Character))
        {
            Character->Destroy();
        }
    }
    GeneratedCharacters.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Cleared all generated characters"));
}

FLinearColor AChar_TribalCharacterManager::GetSkinToneForRole(EChar_TribalRole Role)
{
    switch (Role)
    {
        case EChar_TribalRole::Hunter:
        case EChar_TribalRole::Warrior:
            return FLinearColor(0.7f, 0.5f, 0.3f, 1.0f); // Darker, sun-weathered
        case EChar_TribalRole::Gatherer:
            return FLinearColor(0.8f, 0.6f, 0.4f, 1.0f); // Medium tone
        case EChar_TribalRole::Crafter:
            return FLinearColor(0.85f, 0.65f, 0.45f, 1.0f); // Lighter, indoor work
        case EChar_TribalRole::Elder:
            return FLinearColor(0.75f, 0.55f, 0.35f, 1.0f); // Weathered but not as dark
        case EChar_TribalRole::Shaman:
            return FLinearColor(0.8f, 0.6f, 0.4f, 1.0f); // Medium tone
        default:
            return FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    }
}

float AChar_TribalCharacterManager::GetMuscleMassForRole(EChar_TribalRole Role, bool bIsMale)
{
    float BaseMuscle = bIsMale ? 0.7f : 0.5f;
    
    switch (Role)
    {
        case EChar_TribalRole::Hunter:
        case EChar_TribalRole::Warrior:
            return BaseMuscle + FMath::RandRange(0.1f, 0.3f);
        case EChar_TribalRole::Gatherer:
            return BaseMuscle + FMath::RandRange(0.0f, 0.2f);
        case EChar_TribalRole::Crafter:
            return BaseMuscle + FMath::RandRange(-0.1f, 0.1f);
        case EChar_TribalRole::Elder:
            return BaseMuscle + FMath::RandRange(-0.2f, 0.0f);
        case EChar_TribalRole::Shaman:
            return BaseMuscle + FMath::RandRange(-0.1f, 0.1f);
        default:
            return BaseMuscle;
    }
}

FLinearColor AChar_TribalCharacterManager::GetClothingColorForRole(EChar_TribalRole Role)
{
    switch (Role)
    {
        case EChar_TribalRole::Hunter:
            return FLinearColor(0.3f, 0.25f, 0.15f, 1.0f); // Dark brown, camouflage
        case EChar_TribalRole::Warrior:
            return FLinearColor(0.4f, 0.2f, 0.1f, 1.0f); // Reddish brown, intimidating
        case EChar_TribalRole::Gatherer:
            return FLinearColor(0.5f, 0.4f, 0.2f, 1.0f); // Tan, practical
        case EChar_TribalRole::Crafter:
            return FLinearColor(0.6f, 0.5f, 0.3f, 1.0f); // Lighter brown, clean
        case EChar_TribalRole::Elder:
            return FLinearColor(0.4f, 0.3f, 0.2f, 1.0f); // Dignified dark brown
        case EChar_TribalRole::Shaman:
            return FLinearColor(0.2f, 0.15f, 0.1f, 1.0f); // Very dark, mysterious
        default:
            return FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
    }
}