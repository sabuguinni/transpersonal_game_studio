#include "CharacterArtistSystem.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"

FCharacterArchetypeDefinition UCharacterArchetypeDatabase::GetArchetypeDefinition(ECharacterArchetype ArchetypeType) const
{
    for (const auto& Archetype : Archetypes)
    {
        if (Archetype.ArchetypeType == ArchetypeType)
        {
            return Archetype;
        }
    }
    
    // Return default if not found
    FCharacterArchetypeDefinition DefaultArchetype;
    DefaultArchetype.ArchetypeType = ECharacterArchetype::Protagonist;
    DefaultArchetype.ArchetypeName = TEXT("Default Protagonist");
    return DefaultArchetype;
}

TArray<ECharacterArchetype> UCharacterArchetypeDatabase::GetAvailableArchetypes() const
{
    TArray<ECharacterArchetype> AvailableTypes;
    for (const auto& Archetype : Archetypes)
    {
        AvailableTypes.Add(Archetype.ArchetypeType);
    }
    return AvailableTypes;
}

UCharacterVariationComponent::UCharacterVariationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Default values
    CurrentArchetype = ECharacterArchetype::Protagonist;
    Gender = ECharacterGender::Male;
    Age = ECharacterAge::Adult;
    CharacterInstanceID = GenerateUniqueCharacterID();
}

void UCharacterVariationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find MetaHuman mesh component in owner
    if (AActor* Owner = GetOwner())
    {
        MetaHumanMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
    }
    
    // Apply initial variation
    ApplyVariationToMesh();
}

void UCharacterVariationComponent::GenerateRandomVariation(ECharacterArchetype Archetype)
{
    CurrentArchetype = Archetype;
    
    if (!ArchetypeDatabase.IsNull())
    {
        UCharacterArchetypeDatabase* Database = ArchetypeDatabase.LoadSynchronous();
        if (Database)
        {
            FCharacterArchetypeDefinition ArchetypeDef = Database->GetArchetypeDefinition(Archetype);
            
            // Random gender from preferred list
            if (ArchetypeDef.PreferredGenders.Num() > 0)
            {
                int32 RandomIndex = FMath::RandRange(0, ArchetypeDef.PreferredGenders.Num() - 1);
                Gender = ArchetypeDef.PreferredGenders[RandomIndex];
            }
            
            // Random age from preferred list
            if (ArchetypeDef.PreferredAges.Num() > 0)
            {
                int32 RandomIndex = FMath::RandRange(0, ArchetypeDef.PreferredAges.Num() - 1);
                Age = ArchetypeDef.PreferredAges[RandomIndex];
            }
            
            // Generate random variations within archetype ranges
            VariationData.BodyMassVariation = FMath::FRandRange(
                ArchetypeDef.BaseVariation.BodyMassVariation - ArchetypeDef.VariationRange.BodyMassVariation,
                ArchetypeDef.BaseVariation.BodyMassVariation + ArchetypeDef.VariationRange.BodyMassVariation
            );
            
            VariationData.HeightVariation = FMath::FRandRange(
                ArchetypeDef.BaseVariation.HeightVariation - ArchetypeDef.VariationRange.HeightVariation,
                ArchetypeDef.BaseVariation.HeightVariation + ArchetypeDef.VariationRange.HeightVariation
            );
            
            VariationData.FacialFeatureVariation = FMath::FRandRange(
                ArchetypeDef.BaseVariation.FacialFeatureVariation - ArchetypeDef.VariationRange.FacialFeatureVariation,
                ArchetypeDef.BaseVariation.FacialFeatureVariation + ArchetypeDef.VariationRange.FacialFeatureVariation
            );
            
            // Random skin tone variation
            float SkinVariation = FMath::FRandRange(-0.1f, 0.1f);
            VariationData.SkinTone = ArchetypeDef.BaseVariation.SkinTone;
            VariationData.SkinTone.R = FMath::Clamp(VariationData.SkinTone.R + SkinVariation, 0.0f, 1.0f);
            VariationData.SkinTone.G = FMath::Clamp(VariationData.SkinTone.G + SkinVariation, 0.0f, 1.0f);
            VariationData.SkinTone.B = FMath::Clamp(VariationData.SkinTone.B + SkinVariation, 0.0f, 1.0f);
            
            // Copy clothing and accessories
            VariationData.ClothingPieces = ArchetypeDef.TypicalAccessories;
            
            // Random weathering based on archetype
            VariationData.WeatheringLevel = FMath::FRandRange(
                ArchetypeDef.BaseVariation.WeatheringLevel - 0.2f,
                ArchetypeDef.BaseVariation.WeatheringLevel + 0.2f
            );
            VariationData.WeatheringLevel = FMath::Clamp(VariationData.WeatheringLevel, 0.0f, 1.0f);
        }
    }
    
    ValidateVariationData();
    CharacterInstanceID = GenerateUniqueCharacterID();
}

void UCharacterVariationComponent::ApplyVariationToMesh()
{
    if (!MetaHumanMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterVariationComponent: No MetaHuman mesh found"));
        return;
    }
    
    ApplyPhysicalVariations();
    ApplyClothingAndAccessories();
    ApplyWeatheringAndScars();
    
    UE_LOG(LogTemp, Log, TEXT("Applied character variation for ID: %s"), *CharacterInstanceID);
}

void UCharacterVariationComponent::SetArchetype(ECharacterArchetype NewArchetype)
{
    if (NewArchetype != CurrentArchetype)
    {
        GenerateRandomVariation(NewArchetype);
        ApplyVariationToMesh();
    }
}

FString UCharacterVariationComponent::GenerateUniqueCharacterID() const
{
    FString ArchetypeString = UEnum::GetValueAsString(CurrentArchetype);
    FString GenderString = UEnum::GetValueAsString(Gender);
    FString AgeString = UEnum::GetValueAsString(Age);
    
    // Remove enum prefixes
    ArchetypeString = ArchetypeString.Right(ArchetypeString.Len() - ArchetypeString.Find(TEXT("::")) - 2);
    GenderString = GenderString.Right(GenderString.Len() - GenderString.Find(TEXT("::")) - 2);
    AgeString = AgeString.Right(AgeString.Len() - AgeString.Find(TEXT("::")) - 2);
    
    FString TimeStamp = FString::Printf(TEXT("%d"), FDateTime::Now().GetTicks());
    FString RandomSuffix = FString::Printf(TEXT("%04d"), FMath::RandRange(1000, 9999));
    
    return FString::Printf(TEXT("%s_%s_%s_%s_%s"), 
        *ArchetypeString, 
        *GenderString, 
        *AgeString, 
        *TimeStamp.Right(8), 
        *RandomSuffix
    );
}

void UCharacterVariationComponent::ApplyPhysicalVariations()
{
    if (!MetaHumanMesh) return;
    
    // Apply scale variations
    FVector CurrentScale = MetaHumanMesh->GetRelativeScale3D();
    
    // Height variation
    float HeightMultiplier = 0.9f + (VariationData.HeightVariation * 0.2f); // 0.9 to 1.1
    CurrentScale.Z *= HeightMultiplier;
    
    // Body mass variation affects X and Y scale
    float MassMultiplier = 0.95f + (VariationData.BodyMassVariation * 0.1f); // 0.95 to 1.05
    CurrentScale.X *= MassMultiplier;
    CurrentScale.Y *= MassMultiplier;
    
    MetaHumanMesh->SetRelativeScale3D(CurrentScale);
    
    // TODO: Apply facial feature variations through MetaHuman parameters
    // This would require MetaHuman Creator integration
}

void UCharacterVariationComponent::ApplyClothingAndAccessories()
{
    // TODO: Implement clothing system
    // This would involve:
    // 1. Loading appropriate clothing meshes based on archetype
    // 2. Attaching them to the MetaHuman skeleton
    // 3. Applying appropriate materials and weathering
    
    UE_LOG(LogTemp, Log, TEXT("Applying clothing for archetype: %s"), 
        *UEnum::GetValueAsString(CurrentArchetype));
}

void UCharacterVariationComponent::ApplyWeatheringAndScars()
{
    // TODO: Implement weathering and scar system
    // This would involve:
    // 1. Applying weathering materials based on WeatheringLevel
    // 2. Adding procedural dirt, wear, and aging effects
    // 3. Applying scars and marks from the ScarsAndMarks array
    
    UE_LOG(LogTemp, Log, TEXT("Applying weathering level: %f"), VariationData.WeatheringLevel);
}

void UCharacterVariationComponent::ValidateVariationData()
{
    // Clamp all values to valid ranges
    VariationData.BodyMassVariation = FMath::Clamp(VariationData.BodyMassVariation, 0.0f, 1.0f);
    VariationData.HeightVariation = FMath::Clamp(VariationData.HeightVariation, 0.0f, 1.0f);
    VariationData.FacialFeatureVariation = FMath::Clamp(VariationData.FacialFeatureVariation, 0.0f, 1.0f);
    VariationData.WeatheringLevel = FMath::Clamp(VariationData.WeatheringLevel, 0.0f, 1.0f);
    
    // Ensure color values are valid
    VariationData.SkinTone.R = FMath::Clamp(VariationData.SkinTone.R, 0.0f, 1.0f);
    VariationData.SkinTone.G = FMath::Clamp(VariationData.SkinTone.G, 0.0f, 1.0f);
    VariationData.SkinTone.B = FMath::Clamp(VariationData.SkinTone.B, 0.0f, 1.0f);
    VariationData.SkinTone.A = 1.0f;
}