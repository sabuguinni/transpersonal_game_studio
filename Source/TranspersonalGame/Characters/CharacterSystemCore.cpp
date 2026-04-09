// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "CharacterSystemCore.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimationAsset.h"
#include "Math/UnrealMathUtility.h"

ACharacterSystemCore::ACharacterSystemCore()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize MetaHuman components
    InitializeMetaHumanComponents();
    
    // Set default values
    CharacterArchetype = ECharacterArchetype::Protagonist;
    Ethnicity = ECharacterEthnicity::Mixed;
    CharacterName = TEXT("Dr. Elena Rodriguez");
    BackgroundStory = TEXT("A brilliant paleontologist who discovered a mysterious gem that transported her millions of years into the past.");
    
    // Initialize physical traits with realistic defaults
    PhysicalTraits.Height = 1.0f;
    PhysicalTraits.Weight = 1.0f;
    PhysicalTraits.MuscleMass = 0.4f; // Scientist build
    PhysicalTraits.SkinTone = FLinearColor(0.75f, 0.55f, 0.35f, 1.0f); // Mixed ethnicity
    PhysicalTraits.EyeColor = FLinearColor(0.2f, 0.4f, 0.1f, 1.0f); // Green eyes
    PhysicalTraits.HairColor = FLinearColor(0.15f, 0.1f, 0.05f, 1.0f); // Dark brown
    PhysicalTraits.AgeProgression = 0.3f; // Young adult
    
    // Initialize personality traits for protagonist
    PersonalityTraits.Courage = 0.7f; // Brave enough to explore
    PersonalityTraits.Wisdom = 0.8f; // Scientific knowledge
    PersonalityTraits.Aggression = 0.2f; // Peaceful scientist
    PersonalityTraits.Curiosity = 0.9f; // High scientific curiosity
    PersonalityTraits.Friendliness = 0.6f; // Approachable but cautious
    PersonalityTraits.Leadership = 0.5f; // Natural leader when needed
    PersonalityTraits.SurvivalInstinct = 0.6f; // Learning to survive
    PersonalityTraits.TerritorialBehavior = 0.1f; // Not territorial
}

void ACharacterSystemCore::BeginPlay()
{
    Super::BeginPlay();
    
    // Create dynamic materials for customization
    CreateDynamicMaterials();
    
    // Apply initial character setup
    if (CharacterArchetype == ECharacterArchetype::Protagonist)
    {
        GenerateProtagonist();
    }
    else
    {
        ApplyArchetypeTraits(CharacterArchetype);
        ApplyEthnicityTraits(Ethnicity);
        ApplyCharacterVariation(PhysicalTraits);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Character System Core initialized: %s"), *CharacterName);
}

void ACharacterSystemCore::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update visual elements if needed
    if (bNeedsVisualUpdate)
    {
        UpdateMaterialParameters();
        bNeedsVisualUpdate = false;
    }
}

void ACharacterSystemCore::InitializeMetaHumanComponents()
{
    // Create MetaHuman skeletal mesh components
    MetaHumanBody = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MetaHumanBody"));
    MetaHumanBody->SetupAttachment(GetMesh());
    
    MetaHumanHead = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MetaHumanHead"));
    MetaHumanHead->SetupAttachment(GetMesh(), TEXT("head"));
    
    MetaHumanHair = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MetaHumanHair"));
    MetaHumanHair->SetupAttachment(MetaHumanHead, TEXT("hair_attach"));
    
    // Initialize clothing and equipment arrays
    ClothingPieces.Empty();
    EquipmentPieces.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("MetaHuman components initialized"));
}

void ACharacterSystemCore::CreateDynamicMaterials()
{
    if (MetaHumanBody && MetaHumanBody->GetSkeletalMeshAsset())
    {
        // Create dynamic material for skin
        UMaterialInterface* BaseSkinMaterial = MetaHumanBody->GetMaterial(0);
        if (BaseSkinMaterial)
        {
            SkinMaterial = UMaterialInstanceDynamic::Create(BaseSkinMaterial, this);
            MetaHumanBody->SetMaterial(0, SkinMaterial);
        }
    }
    
    if (MetaHumanHead && MetaHumanHead->GetSkeletalMeshAsset())
    {
        // Create dynamic material for eyes
        UMaterialInterface* BaseEyeMaterial = MetaHumanHead->GetMaterial(1);
        if (BaseEyeMaterial)
        {
            EyeMaterial = UMaterialInstanceDynamic::Create(BaseEyeMaterial, this);
            MetaHumanHead->SetMaterial(1, EyeMaterial);
        }
    }
    
    if (MetaHumanHair && MetaHumanHair->GetSkeletalMeshAsset())
    {
        // Create dynamic material for hair
        UMaterialInterface* BaseHairMaterial = MetaHumanHair->GetMaterial(0);
        if (BaseHairMaterial)
        {
            HairMaterial = UMaterialInstanceDynamic::Create(BaseHairMaterial, this);
            MetaHumanHair->SetMaterial(0, HairMaterial);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dynamic materials created for character customization"));
}

void ACharacterSystemCore::GenerateProtagonist()
{
    // Set protagonist-specific traits
    CharacterArchetype = ECharacterArchetype::Protagonist;
    CharacterName = TEXT("Dr. Elena Rodriguez");
    BackgroundStory = TEXT("A renowned paleontologist who discovered a mysterious crystalline artifact during an excavation. Upon touching it, she was instantly transported millions of years into the past, into a world dominated by dinosaurs and primitive humans. Now she must use her scientific knowledge and adapt to survive in this dangerous prehistoric world while searching for another crystal that might return her home.");
    
    // Set mixed ethnicity with scientific background
    Ethnicity = ECharacterEthnicity::Mixed;
    
    // Physical traits of a scientist
    PhysicalTraits.Height = 0.95f; // Slightly shorter than average
    PhysicalTraits.Weight = 0.85f; // Lean build
    PhysicalTraits.MuscleMass = 0.35f; // Not heavily muscled initially
    PhysicalTraits.SkinTone = FLinearColor(0.72f, 0.52f, 0.32f, 1.0f);
    PhysicalTraits.EyeColor = FLinearColor(0.15f, 0.35f, 0.08f, 1.0f); // Intelligent green
    PhysicalTraits.HairColor = FLinearColor(0.12f, 0.08f, 0.04f, 1.0f); // Dark brown
    PhysicalTraits.AgeProgression = 0.25f; // Young professional
    
    // Protagonist personality
    PersonalityTraits.Courage = 0.75f;
    PersonalityTraits.Wisdom = 0.85f;
    PersonalityTraits.Aggression = 0.15f;
    PersonalityTraits.Curiosity = 0.95f;
    PersonalityTraits.Friendliness = 0.65f;
    PersonalityTraits.Leadership = 0.55f;
    PersonalityTraits.SurvivalInstinct = 0.45f; // Will grow over time
    PersonalityTraits.TerritorialBehavior = 0.05f;
    
    // Apply the traits
    ApplyCharacterVariation(PhysicalTraits);
    
    UE_LOG(LogTemp, Warning, TEXT("Protagonist generated: %s"), *CharacterName);
}

void ACharacterSystemCore::GenerateRandomCharacter()
{
    // Generate random seed for consistent variation
    CharacterSeed = FMath::RandRange(1, 999999);
    FMath::RandInit(CharacterSeed);
    
    // Random archetype (excluding protagonist)
    int32 ArchetypeIndex = FMath::RandRange(1, 7); // Skip protagonist
    CharacterArchetype = static_cast<ECharacterArchetype>(ArchetypeIndex);
    
    // Random ethnicity
    int32 EthnicityIndex = FMath::RandRange(0, 4);
    Ethnicity = static_cast<ECharacterEthnicity>(EthnicityIndex);
    
    // Generate random physical traits
    PhysicalTraits.Height = FMath::RandRange(0.8f, 1.2f);
    PhysicalTraits.Weight = FMath::RandRange(0.7f, 1.3f);
    PhysicalTraits.MuscleMass = FMath::RandRange(0.2f, 0.9f);
    PhysicalTraits.FaceWidth = FMath::RandRange(0.3f, 0.7f);
    PhysicalTraits.JawStrength = FMath::RandRange(0.2f, 0.8f);
    PhysicalTraits.NoseSize = FMath::RandRange(0.3f, 0.7f);
    PhysicalTraits.AgeProgression = FMath::RandRange(0.15f, 0.8f);
    
    // Apply ethnicity-based skin tone
    ApplyEthnicityTraits(Ethnicity);
    
    // Apply archetype-based personality
    ApplyArchetypeTraits(CharacterArchetype);
    
    // Generate random scars based on archetype
    if (CharacterArchetype == ECharacterArchetype::ScarredSurvivor)
    {
        int32 NumScars = FMath::RandRange(2, 5);
        for (int32 i = 0; i < NumScars; i++)
        {
            FVector2D ScarLocation = FVector2D(FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f));
            float ScarIntensity = FMath::RandRange(0.3f, 0.8f);
            AddScar(ScarLocation, ScarIntensity);
        }
    }
    
    // Apply all variations
    ApplyCharacterVariation(PhysicalTraits);
    
    UE_LOG(LogTemp, Log, TEXT("Random character generated with seed: %d"), CharacterSeed);
}

void ACharacterSystemCore::ApplyEthnicityTraits(ECharacterEthnicity TargetEthnicity)
{
    switch (TargetEthnicity)
    {
        case ECharacterEthnicity::African:
            PhysicalTraits.SkinTone = FLinearColor(0.25f, 0.18f, 0.12f, 1.0f);
            PhysicalTraits.EyeColor = FLinearColor(0.08f, 0.05f, 0.03f, 1.0f);
            PhysicalTraits.HairColor = FLinearColor(0.05f, 0.03f, 0.02f, 1.0f);
            break;
            
        case ECharacterEthnicity::Asian:
            PhysicalTraits.SkinTone = FLinearColor(0.85f, 0.75f, 0.55f, 1.0f);
            PhysicalTraits.EyeColor = FLinearColor(0.08f, 0.05f, 0.03f, 1.0f);
            PhysicalTraits.HairColor = FLinearColor(0.05f, 0.03f, 0.02f, 1.0f);
            break;
            
        case ECharacterEthnicity::European:
            PhysicalTraits.SkinTone = FLinearColor(0.95f, 0.85f, 0.75f, 1.0f);
            PhysicalTraits.EyeColor = FLinearColor(0.15f, 0.35f, 0.65f, 1.0f); // Blue eyes
            PhysicalTraits.HairColor = FLinearColor(0.45f, 0.35f, 0.15f, 1.0f); // Blonde
            break;
            
        case ECharacterEthnicity::Indigenous:
            PhysicalTraits.SkinTone = FLinearColor(0.65f, 0.45f, 0.25f, 1.0f);
            PhysicalTraits.EyeColor = FLinearColor(0.08f, 0.05f, 0.03f, 1.0f);
            PhysicalTraits.HairColor = FLinearColor(0.05f, 0.03f, 0.02f, 1.0f);
            break;
            
        case ECharacterEthnicity::Mixed:
        default:
            PhysicalTraits.SkinTone = FLinearColor(0.72f, 0.52f, 0.32f, 1.0f);
            PhysicalTraits.EyeColor = FLinearColor(0.12f, 0.25f, 0.08f, 1.0f);
            PhysicalTraits.HairColor = FLinearColor(0.12f, 0.08f, 0.04f, 1.0f);
            break;
    }
    
    bNeedsVisualUpdate = true;
}

void ACharacterSystemCore::ApplyArchetypeTraits(ECharacterArchetype TargetArchetype)
{
    switch (TargetArchetype)
    {
        case ECharacterArchetype::TribalElder:
            PersonalityTraits.Wisdom = 0.9f;
            PersonalityTraits.Leadership = 0.85f;
            PersonalityTraits.Courage = 0.7f;
            PersonalityTraits.Aggression = 0.3f;
            PhysicalTraits.AgeProgression = 0.75f;
            CharacterName = TEXT("Elder Kahan");
            break;
            
        case ECharacterArchetype::YoungHunter:
            PersonalityTraits.Courage = 0.8f;
            PersonalityTraits.Aggression = 0.6f;
            PersonalityTraits.SurvivalInstinct = 0.85f;
            PhysicalTraits.MuscleMass = 0.8f;
            PhysicalTraits.AgeProgression = 0.2f;
            CharacterName = TEXT("Zara the Swift");
            break;
            
        case ECharacterArchetype::WiseShaman:
            PersonalityTraits.Wisdom = 0.95f;
            PersonalityTraits.Curiosity = 0.8f;
            PersonalityTraits.Friendliness = 0.7f;
            PhysicalTraits.AgeProgression = 0.6f;
            CharacterName = TEXT("Shaman Aiyana");
            break;
            
        case ECharacterArchetype::ScarredSurvivor:
            PersonalityTraits.SurvivalInstinct = 0.95f;
            PersonalityTraits.Courage = 0.9f;
            PersonalityTraits.Aggression = 0.7f;
            PersonalityTraits.Friendliness = 0.3f;
            PhysicalTraits.AgeProgression = 0.45f;
            CharacterName = TEXT("Thane Ironback");
            break;
            
        case ECharacterArchetype::CuriousChild:
            PersonalityTraits.Curiosity = 0.95f;
            PersonalityTraits.Friendliness = 0.8f;
            PersonalityTraits.Courage = 0.4f;
            PhysicalTraits.Height = 0.6f;
            PhysicalTraits.AgeProgression = 0.05f;
            CharacterName = TEXT("Little Naia");
            break;
            
        case ECharacterArchetype::StoicGuardian:
            PersonalityTraits.Courage = 0.9f;
            PersonalityTraits.Leadership = 0.7f;
            PersonalityTraits.Aggression = 0.5f;
            PersonalityTraits.TerritorialBehavior = 0.8f;
            PhysicalTraits.MuscleMass = 0.9f;
            CharacterName = TEXT("Guardian Korak");
            break;
            
        case ECharacterArchetype::MysteriousWanderer:
            PersonalityTraits.Wisdom = 0.7f;
            PersonalityTraits.Curiosity = 0.6f;
            PersonalityTraits.Friendliness = 0.4f;
            PersonalityTraits.TerritorialBehavior = 0.1f;
            CharacterName = TEXT("The Wanderer");
            break;
            
        default:
            break;
    }
}

void ACharacterSystemCore::ApplyCharacterVariation(const FCharacterVariationData& VariationData)
{
    PhysicalTraits = VariationData;
    
    // Apply height and weight scaling
    if (GetMesh())
    {
        FVector CurrentScale = GetMesh()->GetRelativeScale3D();
        CurrentScale.Z = PhysicalTraits.Height;
        CurrentScale.X = CurrentScale.Y = FMath::Lerp(0.9f, 1.1f, PhysicalTraits.Weight);
        GetMesh()->SetRelativeScale3D(CurrentScale);
    }
    
    // Update material parameters
    bNeedsVisualUpdate = true;
    
    UE_LOG(LogTemp, Log, TEXT("Character variation applied to %s"), *CharacterName);
}

void ACharacterSystemCore::UpdateMaterialParameters()
{
    if (SkinMaterial)
    {
        SkinMaterial->SetVectorParameterValue(TEXT("SkinTone"), PhysicalTraits.SkinTone);
        SkinMaterial->SetScalarParameterValue(TEXT("AgeProgression"), PhysicalTraits.AgeProgression);
        SkinMaterial->SetScalarParameterValue(TEXT("MuscleMass"), PhysicalTraits.MuscleMass);
    }
    
    if (EyeMaterial)
    {
        EyeMaterial->SetVectorParameterValue(TEXT("EyeColor"), PhysicalTraits.EyeColor);
    }
    
    if (HairMaterial)
    {
        HairMaterial->SetVectorParameterValue(TEXT("HairColor"), PhysicalTraits.HairColor);
    }
    
    // Apply scars if any
    for (int32 i = 0; i < PhysicalTraits.ScarLocations.Num() && i < PhysicalTraits.ScarIntensities.Num(); i++)
    {
        if (SkinMaterial)
        {
            FString ScarLocationParam = FString::Printf(TEXT("ScarLocation_%d"), i);
            FString ScarIntensityParam = FString::Printf(TEXT("ScarIntensity_%d"), i);
            
            SkinMaterial->SetVectorParameterValue(*ScarLocationParam, 
                FLinearColor(PhysicalTraits.ScarLocations[i].X, PhysicalTraits.ScarLocations[i].Y, 0.0f, 1.0f));
            SkinMaterial->SetScalarParameterValue(*ScarIntensityParam, PhysicalTraits.ScarIntensities[i]);
        }
    }
}

void ACharacterSystemCore::UpdateSkinTone(FLinearColor NewSkinTone)
{
    PhysicalTraits.SkinTone = NewSkinTone;
    bNeedsVisualUpdate = true;
}

void ACharacterSystemCore::UpdateEyeColor(FLinearColor NewEyeColor)
{
    PhysicalTraits.EyeColor = NewEyeColor;
    bNeedsVisualUpdate = true;
}

void ACharacterSystemCore::UpdateHairColor(FLinearColor NewHairColor)
{
    PhysicalTraits.HairColor = NewHairColor;
    bNeedsVisualUpdate = true;
}

void ACharacterSystemCore::AddScar(FVector2D Location, float Intensity)
{
    PhysicalTraits.ScarLocations.Add(Location);
    PhysicalTraits.ScarIntensities.Add(Intensity);
    bNeedsVisualUpdate = true;
    
    UE_LOG(LogTemp, Log, TEXT("Scar added to %s at location (%.2f, %.2f) with intensity %.2f"), 
           *CharacterName, Location.X, Location.Y, Intensity);
}

void ACharacterSystemCore::SetAgeProgression(float AgeValue)
{
    PhysicalTraits.AgeProgression = FMath::Clamp(AgeValue, 0.0f, 1.0f);
    bNeedsVisualUpdate = true;
}

void ACharacterSystemCore::EquipClothingPiece(UStaticMesh* ClothingMesh, FName SocketName)
{
    if (!ClothingMesh || !GetMesh()) return;
    
    UStaticMeshComponent* ClothingComponent = NewObject<UStaticMeshComponent>(this);
    ClothingComponent->SetStaticMesh(ClothingMesh);
    ClothingComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
    
    ClothingPieces.Add(ClothingComponent);
    
    UE_LOG(LogTemp, Log, TEXT("Clothing piece equipped on %s"), *CharacterName);
}

void ACharacterSystemCore::EquipTool(UStaticMesh* ToolMesh, FName SocketName)
{
    if (!ToolMesh || !GetMesh()) return;
    
    UStaticMeshComponent* ToolComponent = NewObject<UStaticMeshComponent>(this);
    ToolComponent->SetStaticMesh(ToolMesh);
    ToolComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
    
    EquipmentPieces.Add(ToolComponent);
    
    UE_LOG(LogTemp, Log, TEXT("Tool equipped on %s"), *CharacterName);
}

void ACharacterSystemCore::RemoveAllEquipment()
{
    for (UStaticMeshComponent* Clothing : ClothingPieces)
    {
        if (Clothing)
        {
            Clothing->DestroyComponent();
        }
    }
    ClothingPieces.Empty();
    
    for (UStaticMeshComponent* Equipment : EquipmentPieces)
    {
        if (Equipment)
        {
            Equipment->DestroyComponent();
        }
    }
    EquipmentPieces.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("All equipment removed from %s"), *CharacterName);
}

void ACharacterSystemCore::ExpressEmotion(const FString& EmotionName, float Intensity)
{
    // This would trigger facial animation blends
    UE_LOG(LogTemp, Log, TEXT("%s expressing %s with intensity %.2f"), *CharacterName, *EmotionName, Intensity);
    
    // TODO: Implement facial animation system
    // This would blend between different facial expressions based on the emotion
}

void ACharacterSystemCore::PlayGesture(const FString& GestureName)
{
    // This would trigger body gesture animations
    UE_LOG(LogTemp, Log, TEXT("%s playing gesture: %s"), *CharacterName, *GestureName);
    
    // TODO: Implement gesture animation system
}

FString ACharacterSystemCore::GetCharacterDescription() const
{
    FString Description = FString::Printf(TEXT("Name: %s\nArchetype: %s\nBackground: %s\n"), 
                                        *CharacterName, 
                                        *UEnum::GetValueAsString(CharacterArchetype),
                                        *BackgroundStory);
    
    Description += FString::Printf(TEXT("Personality - Courage: %.1f, Wisdom: %.1f, Curiosity: %.1f\n"),
                                 PersonalityTraits.Courage,
                                 PersonalityTraits.Wisdom,
                                 PersonalityTraits.Curiosity);
    
    return Description;
}

bool ACharacterSystemCore::IsProtagonist() const
{
    return CharacterArchetype == ECharacterArchetype::Protagonist;
}

float ACharacterSystemCore::GetSurvivalExperience() const
{
    return PersonalityTraits.SurvivalInstinct;
}