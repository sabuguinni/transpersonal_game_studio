#include "Char_PlayerCharacterConfig.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"

UChar_PlayerCharacterConfig::UChar_PlayerCharacterConfig()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Default movement settings for prehistoric survivor
    WalkSpeed = 300.0f;
    RunSpeed = 600.0f;
    JumpHeight = 420.0f;

    // Default camera settings for third-person view
    CameraBoomLength = 400.0f;
    CameraBoomAngle = FRotator(-20.0f, 0.0f, 0.0f);
    CameraFOV = 90.0f;

    // Set default tribal appearance
    SetDefaultTribalAppearance();
}

void UChar_PlayerCharacterConfig::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply configuration to owner character on begin play
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        ApplyAppearanceToCharacter(OwnerCharacter);
        SetDefaultMovementSettings();
    }
}

void UChar_PlayerCharacterConfig::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UChar_PlayerCharacterConfig::ApplyAppearanceToCharacter(ACharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyAppearanceToCharacter: Character is null"));
        return;
    }

    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyAppearanceToCharacter: Character has no mesh component"));
        return;
    }

    // Apply body mesh if available
    if (PlayerAppearance.BodyMesh)
    {
        MeshComp->SetSkeletalMesh(PlayerAppearance.BodyMesh);
        UE_LOG(LogTemp, Log, TEXT("Applied body mesh to character"));
    }

    // Apply skin material if available
    if (PlayerAppearance.SkinMaterial)
    {
        MeshComp->SetMaterial(0, PlayerAppearance.SkinMaterial);
        UE_LOG(LogTemp, Log, TEXT("Applied skin material to character"));
    }

    // Apply clothing mesh as overlay if available
    if (PlayerAppearance.ClothingMesh && PlayerAppearance.ClothingMaterial)
    {
        // Create additional mesh component for clothing
        USkeletalMeshComponent* ClothingComp = NewObject<USkeletalMeshComponent>(Character);
        if (ClothingComp)
        {
            ClothingComp->SetSkeletalMesh(PlayerAppearance.ClothingMesh);
            ClothingComp->SetMaterial(0, PlayerAppearance.ClothingMaterial);
            ClothingComp->SetMasterPoseComponent(MeshComp);
            ClothingComp->AttachToComponent(MeshComp, FAttachmentTransformRules::KeepWorldTransform);
            UE_LOG(LogTemp, Log, TEXT("Applied clothing mesh to character"));
        }
    }

    // Configure camera if character has spring arm component
    TArray<USpringArmComponent*> SpringArms;
    Character->GetComponents<USpringArmComponent>(SpringArms);
    
    if (SpringArms.Num() > 0)
    {
        USpringArmComponent* SpringArm = SpringArms[0];
        SpringArm->TargetArmLength = CameraBoomLength;
        SpringArm->SetRelativeRotation(CameraBoomAngle);
        
        // Find camera component
        TArray<UCameraComponent*> Cameras;
        Character->GetComponents<UCameraComponent>(Cameras);
        
        if (Cameras.Num() > 0)
        {
            UCameraComponent* Camera = Cameras[0];
            Camera->SetFieldOfView(CameraFOV);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Applied camera settings: Boom Length=%.1f, Angle=(%.1f,%.1f,%.1f), FOV=%.1f"), 
               CameraBoomLength, CameraBoomAngle.Pitch, CameraBoomAngle.Yaw, CameraBoomAngle.Roll, CameraFOV);
    }
}

void UChar_PlayerCharacterConfig::SetDefaultTribalAppearance()
{
    // Set default tribal survivor appearance
    PlayerAppearance.SkinTone = FLinearColor(0.75f, 0.55f, 0.35f, 1.0f); // Weathered skin tone
    PlayerAppearance.HairColor = FLinearColor(0.2f, 0.15f, 0.1f, 1.0f);  // Dark brown hair
    
    UE_LOG(LogTemp, Log, TEXT("Set default tribal appearance: Skin(%.2f,%.2f,%.2f), Hair(%.2f,%.2f,%.2f)"),
           PlayerAppearance.SkinTone.R, PlayerAppearance.SkinTone.G, PlayerAppearance.SkinTone.B,
           PlayerAppearance.HairColor.R, PlayerAppearance.HairColor.G, PlayerAppearance.HairColor.B);
}

void UChar_PlayerCharacterConfig::SetDefaultMovementSettings()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetDefaultMovementSettings: Owner is not a character"));
        return;
    }

    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetDefaultMovementSettings: Character has no movement component"));
        return;
    }

    // Apply movement settings
    MovementComp->MaxWalkSpeed = WalkSpeed;
    MovementComp->JumpZVelocity = JumpHeight;
    
    // Set additional prehistoric survivor movement properties
    MovementComp->GroundFriction = 8.0f;           // Higher friction for rough terrain
    MovementComp->BrakingDecelerationWalking = 2048.0f; // Quick stops
    MovementComp->AirControl = 0.2f;               // Limited air control
    MovementComp->Mass = 80.0f;                    // Realistic human mass
    
    UE_LOG(LogTemp, Log, TEXT("Applied movement settings: Walk=%.1f, Run=%.1f, Jump=%.1f"), 
           WalkSpeed, RunSpeed, JumpHeight);
}