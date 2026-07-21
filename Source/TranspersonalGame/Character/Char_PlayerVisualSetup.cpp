#include "Char_PlayerVisualSetup.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

UChar_PlayerVisualSetup::UChar_PlayerVisualSetup()
{
    PrimaryComponentTick.bCanEverTick = false;
    OwnerCharacter = nullptr;

    // Set default mesh path to UE5 mannequin
    MeshConfig.DefaultMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple")));
}

void UChar_PlayerVisualSetup::BeginPlay()
{
    Super::BeginPlay();
    
    CacheOwnerCharacter();
    
    // Auto-setup on begin play
    SetupPlayerVisuals();
}

void UChar_PlayerVisualSetup::CacheOwnerCharacter()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("Char_PlayerVisualSetup: Owner is not a Character"));
    }
}

void UChar_PlayerVisualSetup::SetupPlayerVisuals()
{
    if (!OwnerCharacter)
    {
        CacheOwnerCharacter();
        if (!OwnerCharacter) return;
    }

    // Configure mesh component
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (MeshComp)
    {
        ConfigureMeshComponent(MeshComp);
    }

    // Configure camera system
    USpringArmComponent* CameraBoom = OwnerCharacter->FindComponentByClass<USpringArmComponent>();
    UCameraComponent* FollowCamera = OwnerCharacter->FindComponentByClass<UCameraComponent>();
    if (CameraBoom && FollowCamera)
    {
        ConfigureCameraSystem(CameraBoom, FollowCamera);
    }

    // Configure collision
    UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
    if (CapsuleComp)
    {
        ConfigureCollision(CapsuleComp);
    }

    // Configure movement
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (MovementComp)
    {
        ConfigureMovement(MovementComp);
    }

    UE_LOG(LogTemp, Log, TEXT("Char_PlayerVisualSetup: Player visuals configured"));
}

void UChar_PlayerVisualSetup::ConfigureMeshComponent(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent) return;

    // Load and set skeletal mesh
    if (!MeshConfig.DefaultMesh.IsNull())
    {
        USkeletalMesh* LoadedMesh = MeshConfig.DefaultMesh.LoadSynchronous();
        if (LoadedMesh)
        {
            MeshComponent->SetSkeletalMesh(LoadedMesh);
            UE_LOG(LogTemp, Log, TEXT("Char_PlayerVisualSetup: Set skeletal mesh"));
        }
    }

    // Set mesh transform
    MeshComponent->SetRelativeLocation(MeshConfig.MeshRelativeLocation);
    MeshComponent->SetRelativeRotation(MeshConfig.MeshRelativeRotation);

    // Apply materials if specified
    for (int32 i = 0; i < MeshConfig.DefaultMaterials.Num(); i++)
    {
        if (!MeshConfig.DefaultMaterials[i].IsNull())
        {
            UMaterialInterface* LoadedMaterial = MeshConfig.DefaultMaterials[i].LoadSynchronous();
            if (LoadedMaterial)
            {
                MeshComponent->SetMaterial(i, LoadedMaterial);
            }
        }
    }
}

void UChar_PlayerVisualSetup::ConfigureCameraSystem(USpringArmComponent* CameraBoom, UCameraComponent* FollowCamera)
{
    if (!CameraBoom || !FollowCamera) return;

    // Configure camera boom
    CameraBoom->TargetArmLength = CameraConfig.TargetArmLength;
    CameraBoom->SocketOffset = CameraConfig.SocketOffset;
    CameraBoom->SetRelativeRotation(CameraConfig.BoomRotation);
    CameraBoom->bUsePawnControlRotation = CameraConfig.bUsePawnControlRotation;
    CameraBoom->bInheritPitch = true;
    CameraBoom->bInheritYaw = true;
    CameraBoom->bInheritRoll = false;

    // Configure follow camera
    FollowCamera->SetFieldOfView(CameraConfig.FieldOfView);

    UE_LOG(LogTemp, Log, TEXT("Char_PlayerVisualSetup: Camera system configured"));
}

void UChar_PlayerVisualSetup::ConfigureCollision(UCapsuleComponent* CapsuleComponent)
{
    if (!CapsuleComponent) return;

    CapsuleComponent->SetCapsuleSize(CollisionConfig.CapsuleRadius, CollisionConfig.CapsuleHalfHeight);
    
    UE_LOG(LogTemp, Log, TEXT("Char_PlayerVisualSetup: Collision configured - Radius: %f, Height: %f"), 
           CollisionConfig.CapsuleRadius, CollisionConfig.CapsuleHalfHeight);
}

void UChar_PlayerVisualSetup::ConfigureMovement(UCharacterMovementComponent* MovementComponent)
{
    if (!MovementComponent) return;

    MovementComponent->MaxWalkSpeed = MovementConfig.MaxWalkSpeed;
    MovementComponent->JumpZVelocity = MovementConfig.JumpZVelocity;
    MovementComponent->AirControl = MovementConfig.AirControl;
    MovementComponent->GroundFriction = MovementConfig.GroundFriction;

    // Additional movement settings for better feel
    MovementComponent->BrakingDecelerationWalking = 2048.0f;
    MovementComponent->BrakingFrictionFactor = 2.0f;
    MovementComponent->bCanWalkOffLedges = true;
    MovementComponent->bCanWalkOffLedgesWhenCrouching = true;

    UE_LOG(LogTemp, Log, TEXT("Char_PlayerVisualSetup: Movement configured - Speed: %f, Jump: %f"), 
           MovementConfig.MaxWalkSpeed, MovementConfig.JumpZVelocity);
}

bool UChar_PlayerVisualSetup::ValidateCharacterSetup() const
{
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("Char_PlayerVisualSetup: No owner character"));
        return false;
    }

    // Check mesh component
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp || !MeshComp->GetSkeletalMeshAsset())
    {
        UE_LOG(LogTemp, Warning, TEXT("Char_PlayerVisualSetup: No valid skeletal mesh"));
        return false;
    }

    // Check camera components
    USpringArmComponent* CameraBoom = OwnerCharacter->FindComponentByClass<USpringArmComponent>();
    UCameraComponent* FollowCamera = OwnerCharacter->FindComponentByClass<UCameraComponent>();
    if (!CameraBoom || !FollowCamera)
    {
        UE_LOG(LogTemp, Warning, TEXT("Char_PlayerVisualSetup: Missing camera components"));
        return false;
    }

    // Check collision
    UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
    if (!CapsuleComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Char_PlayerVisualSetup: No capsule component"));
        return false;
    }

    // Check movement
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Char_PlayerVisualSetup: No movement component"));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("Char_PlayerVisualSetup: Character setup validation passed"));
    return true;
}