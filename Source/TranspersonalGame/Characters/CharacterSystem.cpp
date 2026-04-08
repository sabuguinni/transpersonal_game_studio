#include "CharacterSystem.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Math/UnrealMathUtility.h"

UCharacterSystem::UCharacterSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize character variation system
    UE_LOG(LogTemp, Log, TEXT("Character System initialized"));
}

FCharacterVariationData UCharacterSystem::GenerateRandomCharacter(ECharacterArchetype Archetype)
{
    FCharacterVariationData NewCharacter;
    
    // Set archetype
    NewCharacter.Archetype = Archetype;
    
    // Random gender (slightly weighted towards male for prehistoric setting)
    NewCharacter.Gender = (FMath::RandRange(0, 100) < 60) ? ECharacterGender::Male : ECharacterGender::Female;
    
    // Age distribution based on archetype
    switch (Archetype)
    {
        case ECharacterArchetype::TribalChild:
            NewCharacter.AgeGroup = ECharacterAgeGroup::Child;
            break;
        case ECharacterArchetype::TribalElder:
            NewCharacter.AgeGroup = ECharacterAgeGroup::Elder;
            break;
        case ECharacterArchetype::TribalLeader:
        case ECharacterArchetype::TribalShaman:
            // Leaders and shamans tend to be older
            NewCharacter.AgeGroup = (FMath::RandRange(0, 100) < 70) ? ECharacterAgeGroup::MiddleAged : ECharacterAgeGroup::Adult;
            break;
        default:
            // Random adult age for others
            int32 AgeRoll = FMath::RandRange(0, 100);
            if (AgeRoll < 30) NewCharacter.AgeGroup = ECharacterAgeGroup::YoungAdult;
            else if (AgeRoll < 70) NewCharacter.AgeGroup = ECharacterAgeGroup::Adult;
            else NewCharacter.AgeGroup = ECharacterAgeGroup::MiddleAged;
            break;
    }
    
    // Body type based on archetype
    switch (Archetype)
    {
        case ECharacterArchetype::TribalWarrior:
        case ECharacterArchetype::TribalHunter:
            NewCharacter.BodyType = (FMath::RandRange(0, 100) < 80) ? ECharacterBodyType::Athletic : ECharacterBodyType::Lean;
            break;
        case ECharacterArchetype::TribalShaman:
        case ECharacterArchetype::TribalElder:
            NewCharacter.BodyType = (FMath::RandRange(0, 100) < 50) ? ECharacterBodyType::Lean : ECharacterBodyType::Stocky;
            break;
        case ECharacterArchetype::TribalCrafter:
            NewCharacter.BodyType = (FMath::RandRange(0, 100) < 60) ? ECharacterBodyType::Stocky : ECharacterBodyType::Athletic;
            break;
        default:
            // Random distribution for others
            int32 BodyRoll = FMath::RandRange(0, 100);
            if (BodyRoll < 40) NewCharacter.BodyType = ECharacterBodyType::Athletic;
            else if (BodyRoll < 70) NewCharacter.BodyType = ECharacterBodyType::Lean;
            else if (BodyRoll < 90) NewCharacter.BodyType = ECharacterBodyType::Stocky;
            else NewCharacter.BodyType = ECharacterBodyType::Heavy;
            break;
    }
    
    // Generate random MetaHuman variation parameters
    NewCharacter.FaceVariation01 = GetRandomVariationValue();
    NewCharacter.FaceVariation02 = GetRandomVariationValue();
    NewCharacter.FaceVariation03 = GetRandomVariationValue();
    NewCharacter.SkinTone = GetRandomVariationValue();
    NewCharacter.HairVariation = GetRandomVariationValue();
    NewCharacter.EyeColor = GetRandomVariationValue();
    
    // Generate clothing based on archetype
    NewCharacter.ClothingPieces.Add(GetRandomClothingForArchetype(Archetype));
    
    // Generate hair style
    NewCharacter.HairStyle = GetRandomHairStyleForGender(NewCharacter.Gender);
    
    // Generate distinctive features based on archetype and age
    if (NewCharacter.AgeGroup == ECharacterAgeGroup::Elder || NewCharacter.AgeGroup == ECharacterAgeGroup::MiddleAged)
    {
        if (FMath::RandRange(0, 100) < 30)
        {
            NewCharacter.Scars.Add(TEXT("Hunting Scar"));
        }
    }
    
    if (Archetype == ECharacterArchetype::TribalWarrior || Archetype == ECharacterArchetype::TribalLeader)
    {
        if (FMath::RandRange(0, 100) < 60)
        {
            NewCharacter.Tattoos.Add(TEXT("Tribal Markings"));
        }
    }
    
    // Generate character name
    NewCharacter.CharacterName = FString::Printf(TEXT("%s_%s_%d"), 
        *UEnum::GetValueAsString(Archetype).Replace(TEXT("ECharacterArchetype::"), TEXT("")),
        *UEnum::GetValueAsString(NewCharacter.Gender).Replace(TEXT("ECharacterGender::"), TEXT("")),
        FMath::RandRange(1000, 9999));
    
    return NewCharacter;
}

void UCharacterSystem::ApplyCharacterVariation(ACharacter* Character, const FCharacterVariationData& VariationData)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot apply variation to null character"));
        return;
    }
    
    USkeletalMeshComponent* MeshComponent = Character->GetMesh();
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Character has no mesh component"));
        return;
    }
    
    // Apply MetaHuman mesh based on gender
    TArray<USkeletalMesh*>* MeshArray = (VariationData.Gender == ECharacterGender::Male) ? 
        &MaleMetaHumanMeshes : &FemaleMetaHumanMeshes;
    
    if (MeshArray->Num() > 0)
    {
        int32 MeshIndex = FMath::RandRange(0, MeshArray->Num() - 1);
        MeshComponent->SetSkeletalMesh((*MeshArray)[MeshIndex]);
    }
    
    // Apply material parameter variations for MetaHuman customization
    UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
    if (DynamicMaterial)
    {
        DynamicMaterial->SetScalarParameterValue(TEXT("FaceVariation01"), VariationData.FaceVariation01);
        DynamicMaterial->SetScalarParameterValue(TEXT("FaceVariation02"), VariationData.FaceVariation02);
        DynamicMaterial->SetScalarParameterValue(TEXT("FaceVariation03"), VariationData.FaceVariation03);
        DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), VariationData.SkinTone);
        DynamicMaterial->SetScalarParameterValue(TEXT("EyeColor"), VariationData.EyeColor);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied character variation: %s"), *VariationData.CharacterName);
}

TArray<FCharacterVariationData> UCharacterSystem::GetCharactersByArchetype(ECharacterArchetype Archetype)
{
    TArray<FCharacterVariationData> FilteredCharacters;
    
    if (!CharacterVariationTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Character variation table not set"));
        return FilteredCharacters;
    }
    
    TArray<FCharacterVariationData*> AllRows;
    CharacterVariationTable->GetAllRows<FCharacterVariationData>(TEXT("GetCharactersByArchetype"), AllRows);
    
    for (FCharacterVariationData* Row : AllRows)
    {
        if (Row && Row->Archetype == Archetype)
        {
            FilteredCharacters.Add(*Row);
        }
    }
    
    return FilteredCharacters;
}

float UCharacterSystem::GetRandomVariationValue()
{
    // Generate values with slight bias towards center for more natural distribution
    float BaseValue = FMath::RandRange(0.0f, 1.0f);
    float CenterBias = FMath::RandRange(0.0f, 1.0f);
    
    // Blend towards center (0.5) with 30% weight
    return FMath::Lerp(BaseValue, 0.5f, CenterBias * 0.3f);
}

FString UCharacterSystem::GetRandomClothingForArchetype(ECharacterArchetype Archetype)
{
    TArray<FString> ClothingOptions;
    
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist:
            ClothingOptions = {TEXT("Modern Expedition Gear"), TEXT("Torn Modern Clothes"), TEXT("Improvised Prehistoric Clothing")};
            break;
        case ECharacterArchetype::TribalLeader:
            ClothingOptions = {TEXT("Decorated Leather Tunic"), TEXT("Feathered Headdress"), TEXT("Bone Ornaments")};
            break;
        case ECharacterArchetype::TribalWarrior:
            ClothingOptions = {TEXT("Leather Armor"), TEXT("War Paint"), TEXT("Bone Weapons")};
            break;
        case ECharacterArchetype::TribalShaman:
            ClothingOptions = {TEXT("Ritual Robes"), TEXT("Mystical Accessories"), TEXT("Painted Symbols")};
            break;
        case ECharacterArchetype::TribalHunter:
            ClothingOptions = {TEXT("Camouflaged Furs"), TEXT("Hunting Gear"), TEXT("Animal Pelts")};
            break;
        default:
            ClothingOptions = {TEXT("Simple Furs"), TEXT("Woven Grass Clothing"), TEXT("Basic Leather")};
            break;
    }
    
    return ClothingOptions[FMath::RandRange(0, ClothingOptions.Num() - 1)];
}

FString UCharacterSystem::GetRandomHairStyleForGender(ECharacterGender Gender)
{
    TArray<FString> HairStyles;
    
    if (Gender == ECharacterGender::Male)
    {
        HairStyles = {TEXT("Short Messy"), TEXT("Long Braided"), TEXT("Shaved Sides"), TEXT("Wild Mane"), TEXT("Partially Shaved")};
    }
    else
    {
        HairStyles = {TEXT("Long Flowing"), TEXT("Braided Crown"), TEXT("Side Braids"), TEXT("Decorated Bun"), TEXT("Wild Curls")};
    }
    
    return HairStyles[FMath::RandRange(0, HairStyles.Num() - 1)];
}