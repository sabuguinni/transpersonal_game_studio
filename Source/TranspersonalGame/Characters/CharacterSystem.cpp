#include "CharacterSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

// Initialize static member
TArray<FString> UCharacterFactory::MetaHumanPresetDatabase;

ABaseCharacter::ABaseCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    // Set default character type
    CharacterType = ECharacterArchetype::Protagonist;
    CharacterName = TEXT("Dr. Unknown");
    
    // Setup MetaHuman components
    SetupMetaHumanComponents();
}

void ABaseCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial character variation
    ApplyCharacterVariation(CharacterVariation);
}

void ABaseCharacter::SetupMetaHumanComponents()
{
    // Get the default mesh component (this will be the main body)
    MetaHumanBody = GetMesh();
    
    // Create additional MetaHuman components
    MetaHumanHead = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MetaHumanHead"));
    MetaHumanHead->SetupAttachment(MetaHumanBody);
    MetaHumanHead->SetLeaderPoseComponent(MetaHumanBody);
    
    MetaHumanHair = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MetaHumanHair"));
    MetaHumanHair->SetupAttachment(MetaHumanHead);
    MetaHumanHair->SetLeaderPoseComponent(MetaHumanBody);
    
    MetaHumanClothing = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MetaHumanClothing"));
    MetaHumanClothing->SetupAttachment(MetaHumanBody);
    MetaHumanClothing->SetLeaderPoseComponent(MetaHumanBody);
}

void ABaseCharacter::ApplyCharacterVariation(const FCharacterVariation& NewVariation)
{
    CharacterVariation = NewVariation;
    
    // Apply material parameters
    ApplyMaterialParameters();
    
    // Update clothing based on survival state
    UpdateClothingMaterials();
    
    // Update skin based on condition
    UpdateSkinMaterials();
    
    UE_LOG(LogTemp, Log, TEXT("Applied character variation for %s"), *CharacterName);
}

void ABaseCharacter::UpdateSurvivalCondition(int32 NewDaysSurvived)
{
    CharacterVariation.DaysSurvived = NewDaysSurvived;
    
    // Auto-update clothing condition based on days survived
    if (NewDaysSurvived > 90)
    {
        CharacterVariation.ClothingState = EClothingCondition::Primitive;
    }
    else if (NewDaysSurvived > 30)
    {
        CharacterVariation.ClothingState = EClothingCondition::Tattered;
    }
    else if (NewDaysSurvived > 7)
    {
        CharacterVariation.ClothingState = EClothingCondition::Damaged;
    }
    else if (NewDaysSurvived > 1)
    {
        CharacterVariation.ClothingState = EClothingCondition::Worn;
    }
    
    // Increase dirt and scars over time
    CharacterVariation.DirtLevel = FMath::Clamp(NewDaysSurvived / 30.0f, 0.0f, 1.0f);
    CharacterVariation.ScarLevel = FMath::Clamp(NewDaysSurvived / 60.0f, 0.0f, 1.0f);
    
    ApplyCharacterVariation(CharacterVariation);
}

void ABaseCharacter::SetClothingCondition(EClothingCondition NewCondition)
{
    CharacterVariation.ClothingState = NewCondition;
    UpdateClothingMaterials();
}

void ABaseCharacter::SetPhysicalCondition(EPhysicalCondition NewCondition)
{
    CharacterVariation.PhysicalState = NewCondition;
    UpdateSkinMaterials();
}

void ABaseCharacter::ApplyMaterialParameters()
{
    if (!MetaHumanBody) return;
    
    // Create dynamic material instances for body
    for (int32 i = 0; i < MetaHumanBody->GetNumMaterials(); i++)
    {
        UMaterialInterface* BaseMaterial = MetaHumanBody->GetMaterial(i);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            
            // Apply skin tone
            DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), CharacterVariation.SkinTone);
            
            // Apply body weight
            DynamicMaterial->SetScalarParameterValue(TEXT("BodyWeight"), CharacterVariation.BodyWeight);
            
            // Apply muscle definition
            DynamicMaterial->SetScalarParameterValue(TEXT("Muscle"), CharacterVariation.Muscle);
            
            MetaHumanBody->SetMaterial(i, DynamicMaterial);
        }
    }
    
    // Apply hair color if hair component exists
    if (MetaHumanHair)
    {
        for (int32 i = 0; i < MetaHumanHair->GetNumMaterials(); i++)
        {
            UMaterialInterface* BaseMaterial = MetaHumanHair->GetMaterial(i);
            if (BaseMaterial)
            {
                UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
                DynamicMaterial->SetVectorParameterValue(TEXT("HairColor"), CharacterVariation.HairColor);
                MetaHumanHair->SetMaterial(i, DynamicMaterial);
            }
        }
    }
}

void ABaseCharacter::UpdateClothingMaterials()
{
    if (!MetaHumanClothing) return;
    
    for (int32 i = 0; i < MetaHumanClothing->GetNumMaterials(); i++)
    {
        UMaterialInterface* BaseMaterial = MetaHumanClothing->GetMaterial(i);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            
            // Apply clothing wear based on condition
            float WearLevel = 0.0f;
            switch (CharacterVariation.ClothingState)
            {
                case EClothingCondition::Fresh:
                    WearLevel = 0.0f;
                    break;
                case EClothingCondition::Worn:
                    WearLevel = 0.2f;
                    break;
                case EClothingCondition::Damaged:
                    WearLevel = 0.5f;
                    break;
                case EClothingCondition::Tattered:
                    WearLevel = 0.8f;
                    break;
                case EClothingCondition::Primitive:
                    WearLevel = 1.0f;
                    break;
            }
            
            DynamicMaterial->SetScalarParameterValue(TEXT("WearLevel"), WearLevel);
            DynamicMaterial->SetScalarParameterValue(TEXT("DirtLevel"), CharacterVariation.DirtLevel);
            
            MetaHumanClothing->SetMaterial(i, DynamicMaterial);
        }
    }
}

void ABaseCharacter::UpdateSkinMaterials()
{
    if (!MetaHumanBody) return;
    
    for (int32 i = 0; i < MetaHumanBody->GetNumMaterials(); i++)
    {
        UMaterialInterface* BaseMaterial = MetaHumanBody->GetMaterial(i);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            
            // Apply physical condition effects
            float HealthLevel = 1.0f;
            switch (CharacterVariation.PhysicalState)
            {
                case EPhysicalCondition::Healthy:
                    HealthLevel = 1.0f;
                    break;
                case EPhysicalCondition::Tired:
                    HealthLevel = 0.8f;
                    break;
                case EPhysicalCondition::Injured:
                    HealthLevel = 0.6f;
                    break;
                case EPhysicalCondition::Starving:
                    HealthLevel = 0.4f;
                    break;
                case EPhysicalCondition::Sick:
                    HealthLevel = 0.3f;
                    break;
                case EPhysicalCondition::Dying:
                    HealthLevel = 0.1f;
                    break;
            }
            
            DynamicMaterial->SetScalarParameterValue(TEXT("HealthLevel"), HealthLevel);
            DynamicMaterial->SetScalarParameterValue(TEXT("DirtLevel"), CharacterVariation.DirtLevel);
            DynamicMaterial->SetScalarParameterValue(TEXT("ScarLevel"), CharacterVariation.ScarLevel);
            
            MetaHumanBody->SetMaterial(i, DynamicMaterial);
        }
    }
}

// Character Factory Implementation
FCharacterVariation UCharacterFactory::GenerateRandomVariation(ECharacterArchetype ArchetypeHint)
{
    InitializePresetDatabase();
    
    FCharacterVariation NewVariation;
    
    // Select random MetaHuman preset
    if (MetaHumanPresetDatabase.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, MetaHumanPresetDatabase.Num() - 1);
        NewVariation.MetaHumanPresetID = MetaHumanPresetDatabase[RandomIndex];
    }
    
    // Generate random appearance values
    NewVariation.SkinTone = FMath::FRandRange(0.1f, 0.9f);
    NewVariation.BodyWeight = FMath::FRandRange(0.3f, 0.8f);
    NewVariation.Muscle = FMath::FRandRange(0.2f, 0.7f);
    NewVariation.HairStyleID = FMath::RandRange(0, 15);
    
    // Generate hair color
    TArray<FLinearColor> HairColors = {
        FLinearColor::Black,
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f), // Brown
        FLinearColor(0.8f, 0.6f, 0.2f, 1.0f),  // Blonde
        FLinearColor(0.6f, 0.3f, 0.1f, 1.0f),  // Auburn
        FLinearColor(0.5f, 0.5f, 0.5f, 1.0f)   // Gray
    };
    NewVariation.HairColor = HairColors[FMath::RandRange(0, HairColors.Num() - 1)];
    
    // Generate eye color
    TArray<FLinearColor> EyeColors = {
        FLinearColor::Blue,
        FLinearColor::Green,
        FLinearColor(0.4f, 0.2f, 0.1f, 1.0f), // Brown
        FLinearColor(0.3f, 0.3f, 0.3f, 1.0f), // Gray
        FLinearColor(0.2f, 0.4f, 0.2f, 1.0f)  // Hazel
    };
    NewVariation.EyeColor = EyeColors[FMath::RandRange(0, EyeColors.Num() - 1)];
    
    return NewVariation;
}

FCharacterVariation UCharacterFactory::GenerateProtagonistVariation()
{
    FCharacterVariation ProtagonistVariation;
    
    // Protagonist should look like a scientist, not a warrior
    ProtagonistVariation.MetaHumanPresetID = TEXT("Scientist_Male_01");
    ProtagonistVariation.SkinTone = 0.4f; // Slightly pale (indoor work)
    ProtagonistVariation.BodyWeight = 0.6f; // Slightly overweight (desk job)
    ProtagonistVariation.Muscle = 0.3f; // Low muscle (not physical job)
    ProtagonistVariation.HairStyleID = 3; // Professional haircut
    ProtagonistVariation.HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f); // Brown
    ProtagonistVariation.EyeColor = FLinearColor::Blue;
    
    // Starts fresh but will deteriorate
    ProtagonistVariation.ClothingState = EClothingCondition::Fresh;
    ProtagonistVariation.PhysicalState = EPhysicalCondition::Healthy;
    ProtagonistVariation.DirtLevel = 0.0f;
    ProtagonistVariation.ScarLevel = 0.0f;
    ProtagonistVariation.DaysSurvived = 0;
    
    return ProtagonistVariation;
}

FCharacterVariation UCharacterFactory::GenerateSurvivorVariation(int32 DaysSurvived)
{
    FCharacterVariation SurvivorVariation = GenerateRandomVariation(ECharacterArchetype::Survivor);
    
    // Apply survival effects
    SurvivorVariation.DaysSurvived = DaysSurvived;
    SurvivorVariation.DirtLevel = FMath::Clamp(DaysSurvived / 20.0f, 0.2f, 1.0f);
    SurvivorVariation.ScarLevel = FMath::Clamp(DaysSurvived / 40.0f, 0.0f, 0.8f);
    
    // Determine clothing condition based on days survived
    if (DaysSurvived > 60)
    {
        SurvivorVariation.ClothingState = EClothingCondition::Tattered;
    }
    else if (DaysSurvived > 20)
    {
        SurvivorVariation.ClothingState = EClothingCondition::Damaged;
    }
    else if (DaysSurvived > 5)
    {
        SurvivorVariation.ClothingState = EClothingCondition::Worn;
    }
    
    // Physical condition based on survival time
    if (DaysSurvived > 45)
    {
        SurvivorVariation.PhysicalState = EPhysicalCondition::Starving;
    }
    else if (DaysSurvived > 20)
    {
        SurvivorVariation.PhysicalState = EPhysicalCondition::Tired;
    }
    
    return SurvivorVariation;
}

TArray<FString> UCharacterFactory::GetAvailableMetaHumanPresets()
{
    InitializePresetDatabase();
    return MetaHumanPresetDatabase;
}

void UCharacterFactory::InitializePresetDatabase()
{
    if (MetaHumanPresetDatabase.Num() == 0)
    {
        // Initialize with basic MetaHuman preset names
        // These would be replaced with actual MetaHuman preset IDs in production
        MetaHumanPresetDatabase.Add(TEXT("Scientist_Male_01"));
        MetaHumanPresetDatabase.Add(TEXT("Scientist_Female_01"));
        MetaHumanPresetDatabase.Add(TEXT("Explorer_Male_01"));
        MetaHumanPresetDatabase.Add(TEXT("Explorer_Female_01"));
        MetaHumanPresetDatabase.Add(TEXT("Academic_Male_01"));
        MetaHumanPresetDatabase.Add(TEXT("Academic_Female_01"));
        MetaHumanPresetDatabase.Add(TEXT("Researcher_Male_01"));
        MetaHumanPresetDatabase.Add(TEXT("Researcher_Female_01"));
        MetaHumanPresetDatabase.Add(TEXT("Student_Male_01"));
        MetaHumanPresetDatabase.Add(TEXT("Student_Female_01"));
        MetaHumanPresetDatabase.Add(TEXT("Survivor_Male_01"));
        MetaHumanPresetDatabase.Add(TEXT("Survivor_Female_01"));
        MetaHumanPresetDatabase.Add(TEXT("Elderly_Male_01"));
        MetaHumanPresetDatabase.Add(TEXT("Elderly_Female_01"));
        MetaHumanPresetDatabase.Add(TEXT("Young_Male_01"));
        MetaHumanPresetDatabase.Add(TEXT("Young_Female_01"));
    }
}