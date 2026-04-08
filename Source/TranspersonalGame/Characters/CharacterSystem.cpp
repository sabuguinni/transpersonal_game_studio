#include "CharacterSystem.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

void UCharacterManagementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadCharacterDatabase();
}

void UCharacterManagementSubsystem::LoadCharacterDatabase()
{
    // Load character configuration from data asset
    CharacterConfig = LoadObject<UCharacterSystemConfig>(nullptr, TEXT("/Game/Characters/Data/DA_CharacterSystemConfig"));
    
    if (!CharacterConfig.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterSystemConfig not found. Creating default configuration."));
        return;
    }

    // Load character data table
    if (CharacterConfig->CharacterDataTable.IsValid())
    {
        UDataTable* DataTable = CharacterConfig->CharacterDataTable.LoadSynchronous();
        if (DataTable)
        {
            TArray<FCharacterAppearanceData*> AllRows;
            DataTable->GetAllRows<FCharacterAppearanceData>(TEXT("LoadCharacterDatabase"), AllRows);
            
            for (FCharacterAppearanceData* Row : AllRows)
            {
                if (Row)
                {
                    LoadedCharacters.Add(Row->CharacterName, *Row);
                }
            }
            
            UE_LOG(LogTemp, Log, TEXT("Loaded %d characters from database"), LoadedCharacters.Num());
        }
    }
}

FCharacterAppearanceData UCharacterManagementSubsystem::GetCharacterData(const FString& CharacterName)
{
    if (LoadedCharacters.Contains(CharacterName))
    {
        return LoadedCharacters[CharacterName];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Character %s not found in database"), *CharacterName);
    return FCharacterAppearanceData();
}

TArray<FCharacterAppearanceData> UCharacterManagementSubsystem::GetCharactersByArchetype(ECharacterArchetype Archetype)
{
    TArray<FCharacterAppearanceData> Results;
    
    for (const auto& CharacterPair : LoadedCharacters)
    {
        if (CharacterPair.Value.Archetype == Archetype)
        {
            Results.Add(CharacterPair.Value);
        }
    }
    
    return Results;
}

FCharacterAppearanceData UCharacterManagementSubsystem::GenerateRandomCharacter(ECharacterArchetype Archetype)
{
    return CreateRandomAppearanceData(Archetype);
}

FCharacterAppearanceData UCharacterManagementSubsystem::CreateRandomAppearanceData(ECharacterArchetype Archetype)
{
    FCharacterAppearanceData NewCharacter;
    NewCharacter.Archetype = Archetype;
    
    // Random gender
    NewCharacter.bIsMale = FMath::RandBool();
    
    // Random ethnicity with weighted distribution for diversity
    int32 EthnicityRoll = FMath::RandRange(1, 100);
    if (EthnicityRoll <= 20) NewCharacter.Ethnicity = ECharacterEthnicity::African;
    else if (EthnicityRoll <= 35) NewCharacter.Ethnicity = ECharacterEthnicity::Asian;
    else if (EthnicityRoll <= 45) NewCharacter.Ethnicity = ECharacterEthnicity::Indigenous;
    else if (EthnicityRoll <= 55) NewCharacter.Ethnicity = ECharacterEthnicity::MiddleEastern;
    else if (EthnicityRoll <= 70) NewCharacter.Ethnicity = ECharacterEthnicity::Mixed;
    else NewCharacter.Ethnicity = ECharacterEthnicity::European;
    
    // Age distribution based on archetype
    switch (Archetype)
    {
        case ECharacterArchetype::Child:
            NewCharacter.AgeGroup = ECharacterAgeGroup::Child;
            break;
        case ECharacterArchetype::Elder:
            NewCharacter.AgeGroup = ECharacterAgeGroup::Elder;
            break;
        case ECharacterArchetype::Researcher:
            // Researchers tend to be older
            NewCharacter.AgeGroup = FMath::RandBool() ? ECharacterAgeGroup::MiddleAged : ECharacterAgeGroup::Mature;
            break;
        default:
            // Random age for other archetypes
            int32 AgeRoll = FMath::RandRange(1, 100);
            if (AgeRoll <= 5) NewCharacter.AgeGroup = ECharacterAgeGroup::Teenager;
            else if (AgeRoll <= 35) NewCharacter.AgeGroup = ECharacterAgeGroup::YoungAdult;
            else if (AgeRoll <= 65) NewCharacter.AgeGroup = ECharacterAgeGroup::MiddleAged;
            else if (AgeRoll <= 85) NewCharacter.AgeGroup = ECharacterAgeGroup::Mature;
            else NewCharacter.AgeGroup = ECharacterAgeGroup::Elder;
            break;
    }
    
    // Physical variations
    NewCharacter.HeightScale = FMath::RandRange(0.85f, 1.15f);
    NewCharacter.BuildScale = FMath::RandRange(0.9f, 1.1f);
    
    // Survival state based on archetype
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist:
            NewCharacter.DirtinessLevel = FMath::RandRange(0.2f, 0.4f);
            NewCharacter.WearLevel = FMath::RandRange(0.1f, 0.3f);
            NewCharacter.InjuryLevel = FMath::RandRange(0.0f, 0.2f);
            NewCharacter.ConfidenceLevel = FMath::RandRange(0.6f, 0.8f);
            NewCharacter.FearLevel = FMath::RandRange(0.4f, 0.7f);
            break;
        case ECharacterArchetype::Survivor:
            NewCharacter.DirtinessLevel = FMath::RandRange(0.4f, 0.8f);
            NewCharacter.WearLevel = FMath::RandRange(0.5f, 0.9f);
            NewCharacter.InjuryLevel = FMath::RandRange(0.1f, 0.4f);
            NewCharacter.ConfidenceLevel = FMath::RandRange(0.3f, 0.6f);
            NewCharacter.FearLevel = FMath::RandRange(0.6f, 0.9f);
            break;
        case ECharacterArchetype::Elder:
            NewCharacter.DirtinessLevel = FMath::RandRange(0.2f, 0.5f);
            NewCharacter.WearLevel = FMath::RandRange(0.3f, 0.7f);
            NewCharacter.InjuryLevel = FMath::RandRange(0.0f, 0.3f);
            NewCharacter.ConfidenceLevel = FMath::RandRange(0.7f, 0.9f);
            NewCharacter.FearLevel = FMath::RandRange(0.2f, 0.5f);
            break;
        default:
            NewCharacter.DirtinessLevel = FMath::RandRange(0.3f, 0.6f);
            NewCharacter.WearLevel = FMath::RandRange(0.4f, 0.7f);
            NewCharacter.InjuryLevel = FMath::RandRange(0.0f, 0.3f);
            NewCharacter.ConfidenceLevel = FMath::RandRange(0.4f, 0.7f);
            NewCharacter.FearLevel = FMath::RandRange(0.5f, 0.8f);
            break;
    }
    
    // Generate unique name
    TArray<FString> FirstNames = {
        TEXT("Alex"), TEXT("Jordan"), TEXT("Casey"), TEXT("Riley"), TEXT("Morgan"),
        TEXT("Avery"), TEXT("Quinn"), TEXT("Sage"), TEXT("River"), TEXT("Phoenix"),
        TEXT("Kai"), TEXT("Rowan"), TEXT("Ember"), TEXT("Storm"), TEXT("Vale")
    };
    
    NewCharacter.CharacterName = FirstNames[FMath::RandRange(0, FirstNames.Num() - 1)] + 
                                FString::Printf(TEXT("_%d"), FMath::RandRange(1000, 9999));
    
    return NewCharacter;
}

void UCharacterManagementSubsystem::ApplyCharacterAppearance(ACharacter* Character, const FCharacterAppearanceData& AppearanceData)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot apply appearance to null character"));
        return;
    }
    
    USkeletalMeshComponent* MeshComponent = Character->GetMesh();
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Character has no mesh component"));
        return;
    }
    
    // Apply MetaHuman mesh if available
    if (AppearanceData.MetaHumanMesh.IsValid())
    {
        USkeletalMesh* MetaHumanMesh = AppearanceData.MetaHumanMesh.LoadSynchronous();
        if (MetaHumanMesh)
        {
            MeshComponent->SetSkeletalMesh(MetaHumanMesh);
        }
    }
    
    // Apply animation blueprint
    if (AppearanceData.AnimationBlueprint.IsValid())
    {
        UClass* AnimBP = AppearanceData.AnimationBlueprint.LoadSynchronous();
        if (AnimBP)
        {
            MeshComponent->SetAnimInstanceClass(AnimBP);
        }
    }
    
    // Apply scale transformations
    FVector CurrentScale = Character->GetActorScale3D();
    FVector NewScale = FVector(
        CurrentScale.X * AppearanceData.BuildScale,
        CurrentScale.Y * AppearanceData.BuildScale,
        CurrentScale.Z * AppearanceData.HeightScale
    );
    Character->SetActorScale3D(NewScale);
    
    // Apply clothing meshes
    for (int32 i = 0; i < AppearanceData.ClothingMeshes.Num(); ++i)
    {
        if (AppearanceData.ClothingMeshes[i].IsValid())
        {
            USkeletalMesh* ClothingMesh = AppearanceData.ClothingMeshes[i].LoadSynchronous();
            if (ClothingMesh)
            {
                // Create clothing component and attach
                USkeletalMeshComponent* ClothingComponent = NewObject<USkeletalMeshComponent>(Character);
                ClothingComponent->SetSkeletalMesh(ClothingMesh);
                ClothingComponent->SetLeaderPoseComponent(MeshComponent);
                ClothingComponent->AttachToComponent(MeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
                
                // Apply clothing material if available
                if (i < AppearanceData.ClothingMaterials.Num() && AppearanceData.ClothingMaterials[i].IsValid())
                {
                    UMaterialInterface* ClothingMaterial = AppearanceData.ClothingMaterials[i].LoadSynchronous();
                    if (ClothingMaterial)
                    {
                        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(ClothingMaterial, ClothingComponent);
                        
                        // Apply wear and dirt parameters
                        DynamicMaterial->SetScalarParameterValue(TEXT("DirtLevel"), AppearanceData.DirtinessLevel);
                        DynamicMaterial->SetScalarParameterValue(TEXT("WearLevel"), AppearanceData.WearLevel);
                        
                        ClothingComponent->SetMaterial(0, DynamicMaterial);
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied appearance for character: %s"), *AppearanceData.CharacterName);
}