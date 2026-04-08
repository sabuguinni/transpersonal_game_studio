#include "CharacterSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"

ACharacterBase::ACharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create capsule component for collision
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    RootComponent = CapsuleComponent;
    CapsuleComponent->SetCapsuleHalfHeight(88.0f);
    CapsuleComponent->SetCapsuleRadius(34.0f);

    // Create MetaHuman skeletal mesh component
    MetaHumanMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MetaHumanMesh"));
    MetaHumanMesh->SetupAttachment(RootComponent);
    
    // Set default transform for MetaHuman mesh
    MetaHumanMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
    MetaHumanMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
}

void ACharacterBase::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial character data if set
    if (!CharacterData.CharacterName.IsEmpty())
    {
        ApplyCharacterDefinition(CharacterData);
    }
}

void ACharacterBase::ApplyCharacterDefinition(const FCharacterDefinition& NewCharacterData)
{
    CharacterData = NewCharacterData;
    
    // Load and apply MetaHuman mesh
    if (CharacterData.MetaHumanMesh.IsValid())
    {
        USkeletalMesh* LoadedMesh = CharacterData.MetaHumanMesh.LoadSynchronous();
        if (LoadedMesh)
        {
            MetaHumanMesh->SetSkeletalMesh(LoadedMesh);
        }
    }
    
    // Apply physical variations through material parameters
    ApplyPhysicalVariations();
    
    UE_LOG(LogTemp, Log, TEXT("Applied character definition for: %s"), *CharacterData.CharacterName);
}

void ACharacterBase::ApplyPhysicalVariations()
{
    if (!MetaHumanMesh || !MetaHumanMesh->GetSkeletalMeshAsset())
    {
        return;
    }
    
    // Create dynamic material instances for customization
    TArray<UMaterialInterface*> Materials = MetaHumanMesh->GetMaterials();
    
    for (int32 i = 0; i < Materials.Num(); i++)
    {
        if (Materials[i])
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Materials[i], this);
            if (DynamicMaterial)
            {
                // Apply skin tone variation
                DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), CharacterData.PhysicalTraits.SkinTone);
                
                // Apply hair color
                DynamicMaterial->SetVectorParameterValue(TEXT("HairColor"), CharacterData.PhysicalTraits.HairColor);
                
                // Apply eye color
                DynamicMaterial->SetVectorParameterValue(TEXT("EyeColor"), CharacterData.PhysicalTraits.EyeColor);
                
                MetaHumanMesh->SetMaterial(i, DynamicMaterial);
            }
        }
    }
}

void ACharacterBase::GenerateRandomVariation(ECharacterArchetype Archetype, ECharacterEthnicity Ethnicity)
{
    FCharacterVariation NewVariation;
    
    // Generate random physical traits based on ethnicity
    switch (Ethnicity)
    {
        case ECharacterEthnicity::African:
            NewVariation.SkinTone = FMath::RandRange(0.1f, 0.4f);
            NewVariation.HairColor = FLinearColor::Black;
            NewVariation.EyeColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f); // Dark brown
            break;
            
        case ECharacterEthnicity::Asian:
            NewVariation.SkinTone = FMath::RandRange(0.3f, 0.6f);
            NewVariation.HairColor = FLinearColor::Black;
            NewVariation.EyeColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
            NewVariation.EyeSize = FMath::RandRange(0.3f, 0.6f);
            break;
            
        case ECharacterEthnicity::European:
            NewVariation.SkinTone = FMath::RandRange(0.6f, 0.9f);
            NewVariation.HairColor = FLinearColor(FMath::RandRange(0.2f, 0.8f), FMath::RandRange(0.1f, 0.6f), FMath::RandRange(0.0f, 0.3f), 1.0f);
            NewVariation.EyeColor = FLinearColor(FMath::RandRange(0.2f, 0.8f), FMath::RandRange(0.3f, 0.7f), FMath::RandRange(0.4f, 0.9f), 1.0f);
            break;
            
        default:
            NewVariation.SkinTone = FMath::RandRange(0.2f, 0.8f);
            NewVariation.HairColor = FLinearColor(FMath::RandRange(0.1f, 0.9f), FMath::RandRange(0.1f, 0.7f), FMath::RandRange(0.0f, 0.4f), 1.0f);
            NewVariation.EyeColor = FLinearColor(FMath::RandRange(0.2f, 0.8f), FMath::RandRange(0.2f, 0.7f), FMath::RandRange(0.2f, 0.9f), 1.0f);
            break;
    }
    
    // Random facial features
    NewVariation.EyeSize = FMath::RandRange(0.3f, 0.7f);
    NewVariation.NoseSize = FMath::RandRange(0.3f, 0.7f);
    NewVariation.LipFullness = FMath::RandRange(0.3f, 0.7f);
    NewVariation.CheekboneHeight = FMath::RandRange(0.3f, 0.7f);
    NewVariation.JawWidth = FMath::RandRange(0.3f, 0.7f);
    NewVariation.BrowRidge = FMath::RandRange(0.2f, 0.8f);
    
    // Apply archetype-specific variations
    switch (Archetype)
    {
        case ECharacterArchetype::Hunter:
            NewVariation.BrowRidge += 0.2f; // More pronounced features
            NewVariation.JawWidth += 0.1f;
            break;
            
        case ECharacterArchetype::Mystic:
            NewVariation.EyeSize += 0.1f; // Larger, more expressive eyes
            NewVariation.CheekboneHeight += 0.1f;
            break;
            
        case ECharacterArchetype::Elder:
            // Add aging effects through material parameters
            break;
    }
    
    CharacterData.PhysicalTraits = NewVariation;
    ApplyPhysicalVariations();
}