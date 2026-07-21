#include "Char_PlayerCharacterManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"

AChar_PlayerCharacterManager::AChar_PlayerCharacterManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create the character mesh component
    CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
    RootComponent = CharacterMesh;

    // Create the spring arm component for third-person camera
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = CameraBoomLength;
    SpringArm->SetRelativeRotation(CameraBoomRotation);
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bInheritPitch = true;
    SpringArm->bInheritYaw = true;
    SpringArm->bInheritRoll = false;

    // Create the follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Initialize character stats
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    MaxStamina = 80.0f;
    CurrentStamina = MaxStamina;
}

void AChar_PlayerCharacterManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply warrior configuration if asset is set
    if (WarriorAsset)
    {
        ApplyWarriorConfiguration();
    }
    
    // Configure the camera
    ConfigureThirdPersonCamera();
    
    UE_LOG(LogTemp, Log, TEXT("Player Character Manager initialized with health: %.1f, stamina: %.1f"), CurrentHealth, CurrentStamina);
}

void AChar_PlayerCharacterManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Passive stamina regeneration
    if (CurrentStamina < MaxStamina)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (20.0f * DeltaTime));
    }
}

void AChar_PlayerCharacterManager::ApplyWarriorConfiguration()
{
    if (!WarriorAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("AChar_PlayerCharacterManager::ApplyWarriorConfiguration - No warrior asset configured"));
        return;
    }

    // Apply the warrior asset to our character mesh
    WarriorAsset->ApplyToSkeletalMesh(CharacterMesh);

    // Update stats from the asset
    MaxHealth = WarriorAsset->BaseHealth;
    CurrentHealth = MaxHealth;
    MaxStamina = WarriorAsset->BaseStamina;
    CurrentStamina = MaxStamina;

    UE_LOG(LogTemp, Log, TEXT("Applied warrior configuration - Health: %.1f, Stamina: %.1f"), MaxHealth, MaxStamina);
}

void AChar_PlayerCharacterManager::ConfigureThirdPersonCamera()
{
    if (SpringArm)
    {
        SpringArm->TargetArmLength = CameraBoomLength;
        SpringArm->SetRelativeRotation(CameraBoomRotation);
        
        // Enable camera lag for smoother movement
        SpringArm->bEnableCameraLag = true;
        SpringArm->CameraLagSpeed = 3.0f;
        SpringArm->bEnableCameraRotationLag = true;
        SpringArm->CameraRotationLagSpeed = 5.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Configured third-person camera with boom length: %.1f"), CameraBoomLength);
    }

    if (FollowCamera)
    {
        // Set field of view for better gameplay experience
        FollowCamera->SetFieldOfView(90.0f);
        UE_LOG(LogTemp, Log, TEXT("Configured follow camera with FOV: 90 degrees"));
    }
}

void AChar_PlayerCharacterManager::SetHealth(float NewHealth)
{
    CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
    UE_LOG(LogTemp, Log, TEXT("Player health set to: %.1f/%.1f"), CurrentHealth, MaxHealth);
}

void AChar_PlayerCharacterManager::SetStamina(float NewStamina)
{
    CurrentStamina = FMath::Clamp(NewStamina, 0.0f, MaxStamina);
    UE_LOG(LogTemp, Log, TEXT("Player stamina set to: %.1f/%.1f"), CurrentStamina, MaxStamina);
}

float AChar_PlayerCharacterManager::GetHealthPercentage() const
{
    return MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
}

float AChar_PlayerCharacterManager::GetStaminaPercentage() const
{
    return MaxStamina > 0.0f ? (CurrentStamina / MaxStamina) : 0.0f;
}