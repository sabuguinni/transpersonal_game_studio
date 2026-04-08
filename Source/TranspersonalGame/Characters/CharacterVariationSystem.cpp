#include "CharacterVariationSystem.h"
#include "CharacterArchetypes.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UCharacterVariationSystem::UCharacterVariationSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterVariationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize random seed for consistent but varied character generation
    RandomStream.Initialize(FMath::Rand());
}

FCharacterVariationData UCharacterVariationSystem::GenerateRandomVariation(ECharacterArchetype ArchetypeType, ECharacterGender Gender, ECharacterAge Age)
{
    FCharacterVariationData Variation;
    
    // Generate skin tone based on archetype (tribal characters have more varied skin tones)
    switch(ArchetypeType)
    {
        case ECharacterArchetype::Protagonist:
            Variation.SkinTone = RandomStream.FRandRange(0.3f, 0.7f); // Modern person range
            break;
        case ECharacterArchetype::TribalHunter:
        case ECharacterArchetype::TribalShaman:
        case ECharacterArchetype::TribalCrafter:
            Variation.SkinTone = RandomStream.FRandRange(0.2f, 0.9f); // Full range for tribal
            break;
        default:
            Variation.SkinTone = RandomStream.FRandRange(0.25f, 0.85f);
            break;
    }
    
    // Generate hair color
    TArray<FLinearColor> HairColors = {
        FLinearColor(0.05f, 0.05f, 0.05f, 1.0f), // Black
        FLinearColor(0.15f, 0.1f, 0.05f, 1.0f),  // Dark Brown
        FLinearColor(0.3f, 0.2f, 0.1f, 1.0f),    // Brown
        FLinearColor(0.6f, 0.4f, 0.2f, 1.0f),    // Light Brown
        FLinearColor(0.8f, 0.6f, 0.3f, 1.0f),    // Blonde
        FLinearColor(0.4f, 0.15f, 0.1f, 1.0f),   // Auburn
        FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)     // Gray (for elders)
    };
    
    int32 HairColorIndex = RandomStream.RandRange(0, HairColors.Num() - 1);
    if(Age == ECharacterAge::Elder)
    {
        // Elders more likely to have gray hair
        HairColorIndex = RandomStream.FRand() < 0.6f ? HairColors.Num() - 1 : HairColorIndex;
    }
    Variation.HairColor = HairColors[HairColorIndex];
    
    // Generate eye color
    TArray<FLinearColor> EyeColors = {
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f),   // Brown
        FLinearColor(0.1f, 0.3f, 0.6f, 1.0f),    // Blue
        FLinearColor(0.2f, 0.4f, 0.2f, 1.0f),    // Green
        FLinearColor(0.3f, 0.25f, 0.1f, 1.0f),   // Hazel
        FLinearColor(0.15f, 0.15f, 0.15f, 1.0f)  // Gray
    };
    Variation.EyeColor = EyeColors[RandomStream.RandRange(0, EyeColors.Num() - 1)];
    
    // Generate height and weight based on age and gender
    float BaseHeight = 1.0f;
    float BaseWeight = 1.0f;
    
    switch(Age)
    {
        case ECharacterAge::Child:
            BaseHeight = RandomStream.FRandRange(0.5f, 0.7f);
            BaseWeight = RandomStream.FRandRange(0.6f, 0.8f);
            break;
        case ECharacterAge::Teen:
            BaseHeight = RandomStream.FRandRange(0.8f, 0.95f);
            BaseWeight = RandomStream.FRandRange(0.7f, 0.9f);
            break;
        case ECharacterAge::Young:
            BaseHeight = RandomStream.FRandRange(0.9f, 1.1f);
            BaseWeight = RandomStream.FRandRange(0.8f, 1.2f);
            break;
        case ECharacterAge::Adult:
            BaseHeight = RandomStream.FRandRange(0.95f, 1.05f);
            BaseWeight = RandomStream.FRandRange(0.9f, 1.3f);
            break;
        case ECharacterAge::Elder:
            BaseHeight = RandomStream.FRandRange(0.9f, 1.0f); // Slight shrinkage
            BaseWeight = RandomStream.FRandRange(0.8f, 1.1f);
            break;
    }
    
    // Adjust for gender
    if(Gender == ECharacterGender::Female)
    {
        BaseHeight *= RandomStream.FRandRange(0.92f, 0.98f); // Slightly shorter on average
    }
    
    Variation.Height = BaseHeight;
    Variation.Weight = BaseWeight;
    
    // Generate distinguishing features based on archetype and age
    switch(ArchetypeType)
    {
        case ECharacterArchetype::TribalHunter:
            Variation.bHasScars = RandomStream.FRand() < 0.7f; // High chance of scars
            Variation.bHasTattoos = RandomStream.FRand() < 0.8f; // High chance of tattoos
            break;
        case ECharacterArchetype::TribalShaman:
            Variation.bHasTattoos = RandomStream.FRand() < 0.9f; // Very high chance of tattoos
            Variation.bHasScars = RandomStream.FRand() < 0.3f;
            break;
        case ECharacterArchetype::WildHermit:
            Variation.bHasScars = RandomStream.FRand() < 0.8f; // Many scars from survival
            break;
        default:
            Variation.bHasScars = RandomStream.FRand() < 0.2f;
            Variation.bHasTattoos = RandomStream.FRand() < 0.1f;
            break;
    }
    
    // Beard generation for males
    if(Gender == ECharacterGender::Male && Age != ECharacterAge::Child)
    {
        float BeardChance = 0.4f;
        if(ArchetypeType == ECharacterArchetype::WildHermit) BeardChance = 0.9f;
        if(ArchetypeType == ECharacterArchetype::TribalElder) BeardChance = 0.7f;
        if(Age == ECharacterAge::Elder) BeardChance += 0.3f;
        
        Variation.bHasBeard = RandomStream.FRand() < BeardChance;
    }
    
    // Generate pattern indices
    Variation.ScarPattern = RandomStream.RandRange(0, 4); // Assuming 5 scar patterns
    Variation.TattooPattern = RandomStream.RandRange(0, 7); // Assuming 8 tattoo patterns
    
    return Variation;
}

void UCharacterVariationSystem::ApplyVariationToMetaHuman(USkeletalMeshComponent* MetaHumanMesh, const FCharacterVariationData& Variation)
{
    if(!MetaHumanMesh || !MetaHumanMesh->GetSkeletalMeshAsset())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid MetaHuman mesh component"));
        return;
    }
    
    // Create dynamic material instances for customization
    TArray<UMaterialInterface*> Materials = MetaHumanMesh->GetMaterials();
    
    for(int32 i = 0; i < Materials.Num(); i++)
    {
        if(UMaterialInterface* Material = Materials[i])
        {
            UMaterialInstanceDynamic* DynMaterial = MetaHumanMesh->CreateDynamicMaterialInstance(i, Material);
            if(DynMaterial)
            {
                // Apply skin tone
                DynMaterial->SetScalarParameterValue(TEXT("Skin_Tone"), Variation.SkinTone);
                
                // Apply hair color
                DynMaterial->SetVectorParameterValue(TEXT("Hair_Color"), Variation.HairColor);
                
                // Apply eye color
                DynMaterial->SetVectorParameterValue(TEXT("Eye_Color"), Variation.EyeColor);
                
                // Apply scar visibility
                DynMaterial->SetScalarParameterValue(TEXT("Scar_Visibility"), Variation.bHasScars ? 1.0f : 0.0f);
                DynMaterial->SetScalarParameterValue(TEXT("Scar_Pattern"), Variation.ScarPattern);
                
                // Apply tattoo visibility
                DynMaterial->SetScalarParameterValue(TEXT("Tattoo_Visibility"), Variation.bHasTattoos ? 1.0f : 0.0f);
                DynMaterial->SetScalarParameterValue(TEXT("Tattoo_Pattern"), Variation.TattooPattern);
                
                // Apply beard visibility (if applicable)
                DynMaterial->SetScalarParameterValue(TEXT("Beard_Visibility"), Variation.bHasBeard ? 1.0f : 0.0f);
            }
        }
    }
    
    // Apply scale transformations
    FVector CurrentScale = MetaHumanMesh->GetRelativeScale3D();
    FVector NewScale = FVector(
        CurrentScale.X,
        CurrentScale.Y * Variation.Weight,
        CurrentScale.Z * Variation.Height
    );
    MetaHumanMesh->SetRelativeScale3D(NewScale);
}

FString UCharacterVariationSystem::GenerateCharacterID(ECharacterArchetype ArchetypeType, const FCharacterVariationData& Variation)
{
    // Create a unique ID based on archetype and key variation data
    FString ArchetypeString = UEnum::GetValueAsString(ArchetypeType);
    ArchetypeString = ArchetypeString.Replace(TEXT("ECharacterArchetype::"), TEXT(""));
    
    // Create hash from variation data for uniqueness
    uint32 VariationHash = GetTypeHash(Variation.SkinTone) ^ 
                          GetTypeHash(Variation.HairColor.ToString()) ^
                          GetTypeHash(Variation.Height) ^
                          GetTypeHash(Variation.bHasScars) ^
                          GetTypeHash(Variation.ScarPattern);
    
    return FString::Printf(TEXT("%s_%08X"), *ArchetypeString, VariationHash);
}