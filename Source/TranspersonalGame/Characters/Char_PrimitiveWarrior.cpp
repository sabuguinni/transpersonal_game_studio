#include "Char_PrimitiveWarrior.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

AChar_PrimitiveWarrior::AChar_PrimitiveWarrior()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set default character properties
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
    GetCapsuleComponent()->SetCapsuleRadius(34.0f);

    // Configure character movement
    GetCharacterMovement()->MaxWalkSpeed = 500.0f;
    GetCharacterMovement()->JumpZVelocity = 400.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

    // Create tribal equipment components
    SpearMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpearMesh"));
    SpearMesh->SetupAttachment(GetMesh(), TEXT("hand_r"));
    SpearMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    SpearMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

    BoneNecklaceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoneNecklaceMesh"));
    BoneNecklaceMesh->SetupAttachment(GetMesh(), TEXT("neck_01"));
    BoneNecklaceMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

    StoneKnifeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StoneKnifeMesh"));
    StoneKnifeMesh->SetupAttachment(GetMesh(), TEXT("thigh_l"));
    StoneKnifeMesh->SetRelativeLocation(FVector(5.0f, 0.0f, 0.0f));

    // Initialize tribal stats
    TribalStrength = 75.0f;
    SurvivalExperience = 60.0f;
    HuntingSkill = 80.0f;

    // Set default mesh and materials to null - will be set up in BeginPlay
    SkinMaterial = nullptr;
    ClothingMaterial = nullptr;
    SpearMeshAsset = nullptr;
    BoneNecklaceAsset = nullptr;
    StoneKnifeAsset = nullptr;
}

void AChar_PrimitiveWarrior::BeginPlay()
{
    Super::BeginPlay();
    
    SetupTribalAppearance();
    AttachTribalEquipment();
}

void AChar_PrimitiveWarrior::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update tribal behavior or animations here if needed
}

void AChar_PrimitiveWarrior::SetupTribalAppearance()
{
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        // Apply skin material if available
        if (SkinMaterial)
        {
            MeshComp->SetMaterial(0, SkinMaterial);
        }
        
        // Apply clothing material if available
        if (ClothingMaterial)
        {
            MeshComp->SetMaterial(1, ClothingMaterial);
        }
        
        // Set mesh to use a more primitive/weathered look
        MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
        MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }
}

void AChar_PrimitiveWarrior::AttachTribalEquipment()
{
    // Attach spear mesh
    if (SpearMesh && SpearMeshAsset)
    {
        SpearMesh->SetStaticMesh(SpearMeshAsset);
        SpearMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    // Attach bone necklace
    if (BoneNecklaceMesh && BoneNecklaceAsset)
    {
        BoneNecklaceMesh->SetStaticMesh(BoneNecklaceAsset);
        BoneNecklaceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    // Attach stone knife
    if (StoneKnifeMesh && StoneKnifeAsset)
    {
        StoneKnifeMesh->SetStaticMesh(StoneKnifeAsset);
        StoneKnifeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void AChar_PrimitiveWarrior::SetTribalStats(float Strength, float Experience, float Hunting)
{
    TribalStrength = FMath::Clamp(Strength, 0.0f, 100.0f);
    SurvivalExperience = FMath::Clamp(Experience, 0.0f, 100.0f);
    HuntingSkill = FMath::Clamp(Hunting, 0.0f, 100.0f);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Tribal Stats - Strength: %.1f, Experience: %.1f, Hunting: %.1f"), 
                TribalStrength, SurvivalExperience, HuntingSkill));
    }
}