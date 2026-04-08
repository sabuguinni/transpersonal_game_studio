#include "TranspersonalCharacterBase.h"
#include "CharacterVariationComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

ATranspersonalCharacterBase::ATranspersonalCharacterBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set default character name
    CharacterName = TEXT("Unknown Character");
    CharacterArchetype = TEXT("Default");

    // Create variation component
    VariationComponent = CreateDefaultSubobject<UCharacterVariationComponent>(TEXT("VariationComponent"));

    // Initialize MetaHuman components
    InitializeMetaHumanComponents();

    // Configure character movement for survival gameplay
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->MaxAcceleration = 1024.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 1024.0f;
    GetCharacterMovement()->GroundFriction = 8.0f;
    GetCharacterMovement()->JumpZVelocity = 500.0f;
    GetCharacterMovement()->AirControl = 0.2f;

    // Set collision
    GetCapsuleComponent()->SetCapsuleSize(42.0f, 96.0f);
}

void ATranspersonalCharacterBase::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply any initial character variation
    if (!VariationHash.IsEmpty())
    {
        ApplyCharacterVariation();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Character '%s' (%s) initialized"), *CharacterName, *CharacterArchetype);
}

void ATranspersonalCharacterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update survival stats if this is the protagonist
    if (bIsProtagonist)
    {
        // Gradual hunger and thirst decrease (survival mechanics)
        Hunger = FMath::Clamp(Hunger - (DeltaTime * 0.1f), 0.0f, 100.0f);
        Thirst = FMath::Clamp(Thirst - (DeltaTime * 0.15f), 0.0f, 100.0f);
        
        // Health effects from hunger/thirst
        if (Hunger <= 10.0f || Thirst <= 10.0f)
        {
            Health = FMath::Clamp(Health - (DeltaTime * 2.0f), 0.0f, MaxHealth);
        }
    }
}

void ATranspersonalCharacterBase::SetCharacterName(const FString& NewName)
{
    CharacterName = NewName;
    UE_LOG(LogTemp, Log, TEXT("Character name set to: %s"), *CharacterName);
}

void ATranspersonalCharacterBase::SetCharacterArchetype(const FString& NewArchetype)
{
    CharacterArchetype = NewArchetype;
    UE_LOG(LogTemp, Log, TEXT("Character archetype set to: %s"), *CharacterArchetype);
}

void ATranspersonalCharacterBase::SetIsProtagonist(bool bNewIsProtagonist)
{
    bIsProtagonist = bNewIsProtagonist;
    
    if (bIsProtagonist)
    {
        // Configure protagonist-specific settings
        Tags.AddUnique(TEXT("Protagonist"));
        UE_LOG(LogTemp, Log, TEXT("Character '%s' set as protagonist"), *CharacterName);
    }
}

void ATranspersonalCharacterBase::SetCharacterVariationHash(const FString& NewHash)
{
    VariationHash = NewHash;
    UE_LOG(LogTemp, Log, TEXT("Character variation hash set: %s"), *VariationHash);
}

void ATranspersonalCharacterBase::ApplyCharacterVariation()
{
    if (VariationComponent)
    {
        VariationComponent->ApplyVariationFromHash(VariationHash);
        ApplyMetaHumanVariation();
        UE_LOG(LogTemp, Log, TEXT("Applied character variation for: %s"), *CharacterName);
    }
}

void ATranspersonalCharacterBase::SetMetaHumanAsset(UMetaHumanCharacterAsset* NewAsset)
{
    // This will be implemented when MetaHuman integration is complete
    UE_LOG(LogTemp, Log, TEXT("MetaHuman asset set for character: %s"), *CharacterName);
}

void ATranspersonalCharacterBase::LoadMetaHumanFromPreset(const FString& PresetName)
{
    // Load MetaHuman preset by name
    UE_LOG(LogTemp, Log, TEXT("Loading MetaHuman preset '%s' for character: %s"), *PresetName, *CharacterName);
    
    // This will be implemented with actual MetaHuman loading logic
}

void ATranspersonalCharacterBase::InitializeMetaHumanComponents()
{
    // Create additional skeletal mesh components for MetaHuman structure
    BodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
    FaceMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FaceMesh"));
    HairMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh"));

    if (BodyMesh)
    {
        BodyMesh->SetupAttachment(GetMesh());
        BodyMesh->SetLeaderPoseComponent(GetMesh());
    }

    if (FaceMesh)
    {
        FaceMesh->SetupAttachment(GetMesh());
        FaceMesh->SetLeaderPoseComponent(GetMesh());
    }

    if (HairMesh)
    {
        HairMesh->SetupAttachment(GetMesh());
        HairMesh->SetLeaderPoseComponent(GetMesh());
    }
}

void ATranspersonalCharacterBase::ApplyMetaHumanVariation()
{
    // Apply variation to MetaHuman components
    // This will contain the actual MetaHuman parameter adjustments
    UE_LOG(LogTemp, Log, TEXT("Applying MetaHuman variation for: %s"), *CharacterName);
}