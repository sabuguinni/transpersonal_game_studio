#include "Char_PlayerVisibilityManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/SoftObjectPtr.h"

UChar_PlayerVisibilityManager::UChar_PlayerVisibilityManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    // Default camera settings for third-person view
    DefaultCameraArmLength = 400.0f;
    DefaultCameraOffset = FVector(0.0f, 0.0f, 50.0f);
    DefaultCameraRotation = FRotator(-15.0f, 0.0f, 0.0f);
    
    // Visibility settings
    bEnsureVisibleMesh = true;
    bAutoApplyFallback = true;
    MeshValidationInterval = 2.0f;
    
    // Initialize state
    CachedMeshComponent = nullptr;
    CachedCameraBoom = nullptr;
    CachedCamera = nullptr;
    LastValidationTime = 0.0f;
    bMeshSetupComplete = false;
    
    // Set up fallback mesh paths (common UE5 meshes)
    FallbackMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath("/Engine/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
    
    // Add alternative fallback meshes
    AvailableMeshes.Add(TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple")));
    AvailableMeshes.Add(TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath("/Engine/Characters/Mannequins/Meshes/SKM_Manny_Simple")));
    AvailableMeshes.Add(FallbackMesh);
}

void UChar_PlayerVisibilityManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache component references
    CachedMeshComponent = GetPlayerMeshComponent();
    CachedCameraBoom = GetCameraBoom();
    CachedCamera = GetCamera();
    
    // Initial setup
    SetupPlayerMesh();
    ConfigureThirdPersonCamera();
    
    UE_LOG(LogTemp, Warning, TEXT("PlayerVisibilityManager: BeginPlay complete"));
}

void UChar_PlayerVisibilityManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic validation
    LastValidationTime += DeltaTime;
    if (LastValidationTime >= MeshValidationInterval)
    {
        if (bEnsureVisibleMesh && !ValidateMeshVisibility())
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerVisibilityManager: Mesh validation failed, applying fallback"));
            ApplyFallbackMesh();
        }
        LastValidationTime = 0.0f;
    }
}

void UChar_PlayerVisibilityManager::SetupPlayerMesh()
{
    USkeletalMeshComponent* MeshComp = GetPlayerMeshComponent();
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerVisibilityManager: No SkeletalMeshComponent found"));
        return;
    }
    
    // Try preferred mesh first
    if (PreferredPlayerMesh.IsValid())
    {
        USkeletalMesh* LoadedMesh = PreferredPlayerMesh.LoadSynchronous();
        if (LoadedMesh)
        {
            MeshComp->SetSkeletalMesh(LoadedMesh);
            bMeshSetupComplete = true;
            LogMeshStatus("Preferred mesh applied");
            return;
        }
    }
    
    // Try available meshes
    for (const auto& MeshPtr : AvailableMeshes)
    {
        if (TryLoadMesh(MeshPtr))
        {
            bMeshSetupComplete = true;
            LogMeshStatus("Available mesh applied");
            return;
        }
    }
    
    // Apply fallback as last resort
    ApplyFallbackMesh();
}

void UChar_PlayerVisibilityManager::ApplyFallbackMesh()
{
    USkeletalMeshComponent* MeshComp = GetPlayerMeshComponent();
    if (!MeshComp)
    {
        return;
    }
    
    // Try the designated fallback mesh
    if (TryLoadMesh(FallbackMesh))
    {
        bMeshSetupComplete = true;
        LogMeshStatus("Fallback mesh applied");
        return;
    }
    
    // Create a basic capsule representation if all else fails
    UE_LOG(LogTemp, Error, TEXT("PlayerVisibilityManager: All mesh loading failed, character may be invisible"));
    bMeshSetupComplete = false;
}

void UChar_PlayerVisibilityManager::SetCustomMesh(USkeletalMesh* NewMesh)
{
    if (!NewMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerVisibilityManager: Attempted to set null custom mesh"));
        return;
    }
    
    USkeletalMeshComponent* MeshComp = GetPlayerMeshComponent();
    if (MeshComp)
    {
        MeshComp->SetSkeletalMesh(NewMesh);
        bMeshSetupComplete = true;
        LogMeshStatus("Custom mesh applied");
    }
}

bool UChar_PlayerVisibilityManager::ValidateMeshVisibility()
{
    USkeletalMeshComponent* MeshComp = GetPlayerMeshComponent();
    if (!MeshComp)
    {
        return false;
    }
    
    USkeletalMesh* CurrentMesh = MeshComp->GetSkeletalMeshAsset();
    if (!CurrentMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerVisibilityManager: No skeletal mesh assigned"));
        return false;
    }
    
    // Check if mesh is visible
    if (!MeshComp->IsVisible())
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerVisibilityManager: Mesh component is not visible"));
        return false;
    }
    
    return true;
}

void UChar_PlayerVisibilityManager::ConfigureThirdPersonCamera()
{
    USpringArmComponent* CameraBoom = GetCameraBoom();
    UCameraComponent* Camera = GetCamera();
    
    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = DefaultCameraArmLength;
        CameraBoom->SocketOffset = DefaultCameraOffset;
        CameraBoom->SetRelativeRotation(DefaultCameraRotation);
        CameraBoom->bUsePawnControlRotation = true;
        CameraBoom->bInheritPitch = true;
        CameraBoom->bInheritYaw = true;
        CameraBoom->bInheritRoll = false;
        
        UE_LOG(LogTemp, Log, TEXT("PlayerVisibilityManager: Camera boom configured"));
    }
    
    if (Camera)
    {
        Camera->bUsePawnControlRotation = false; // Camera follows boom
        UE_LOG(LogTemp, Log, TEXT("PlayerVisibilityManager: Camera configured"));
    }
}

void UChar_PlayerVisibilityManager::AdjustCameraBoom(float ArmLength, FVector SocketOffset)
{
    USpringArmComponent* CameraBoom = GetCameraBoom();
    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = ArmLength;
        CameraBoom->SocketOffset = SocketOffset;
        
        UE_LOG(LogTemp, Log, TEXT("PlayerVisibilityManager: Camera boom adjusted - Arm: %f, Offset: %s"), 
               ArmLength, *SocketOffset.ToString());
    }
}

void UChar_PlayerVisibilityManager::ApplyBaseMaterials()
{
    USkeletalMeshComponent* MeshComp = GetPlayerMeshComponent();
    if (!MeshComp)
    {
        return;
    }
    
    // Apply default skin material
    if (DefaultSkinMaterial.IsValid())
    {
        UMaterialInterface* SkinMat = DefaultSkinMaterial.LoadSynchronous();
        if (SkinMat)
        {
            MeshComp->SetMaterial(0, SkinMat); // Assume slot 0 is skin
        }
    }
    
    // Apply default clothing material
    if (DefaultClothingMaterial.IsValid())
    {
        UMaterialInterface* ClothingMat = DefaultClothingMaterial.LoadSynchronous();
        if (ClothingMat)
        {
            MeshComp->SetMaterial(1, ClothingMat); // Assume slot 1 is clothing
        }
    }
}

void UChar_PlayerVisibilityManager::SetSkinMaterial(UMaterialInterface* SkinMaterial)
{
    if (!SkinMaterial)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = GetPlayerMeshComponent();
    if (MeshComp)
    {
        MeshComp->SetMaterial(0, SkinMaterial);
        UE_LOG(LogTemp, Log, TEXT("PlayerVisibilityManager: Skin material applied"));
    }
}

void UChar_PlayerVisibilityManager::SetClothingMaterial(UMaterialInterface* ClothingMaterial)
{
    if (!ClothingMaterial)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = GetPlayerMeshComponent();
    if (MeshComp)
    {
        MeshComp->SetMaterial(1, ClothingMaterial);
        UE_LOG(LogTemp, Log, TEXT("PlayerVisibilityManager: Clothing material applied"));
    }
}

USkeletalMeshComponent* UChar_PlayerVisibilityManager::GetPlayerMeshComponent()
{
    if (CachedMeshComponent)
    {
        return CachedMeshComponent;
    }
    
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        CachedMeshComponent = OwnerCharacter->GetMesh();
    }
    
    return CachedMeshComponent;
}

USpringArmComponent* UChar_PlayerVisibilityManager::GetCameraBoom()
{
    if (CachedCameraBoom)
    {
        return CachedCameraBoom;
    }
    
    if (GetOwner())
    {
        CachedCameraBoom = GetOwner()->FindComponentByClass<USpringArmComponent>();
    }
    
    return CachedCameraBoom;
}

UCameraComponent* UChar_PlayerVisibilityManager::GetCamera()
{
    if (CachedCamera)
    {
        return CachedCamera;
    }
    
    if (GetOwner())
    {
        CachedCamera = GetOwner()->FindComponentByClass<UCameraComponent>();
    }
    
    return CachedCamera;
}

bool UChar_PlayerVisibilityManager::TryLoadMesh(const TSoftObjectPtr<USkeletalMesh>& MeshPtr)
{
    if (!MeshPtr.IsValid())
    {
        return false;
    }
    
    USkeletalMesh* LoadedMesh = MeshPtr.LoadSynchronous();
    if (!LoadedMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerVisibilityManager: Failed to load mesh: %s"), 
               *MeshPtr.ToString());
        return false;
    }
    
    USkeletalMeshComponent* MeshComp = GetPlayerMeshComponent();
    if (MeshComp)
    {
        MeshComp->SetSkeletalMesh(LoadedMesh);
        return true;
    }
    
    return false;
}

void UChar_PlayerVisibilityManager::LogMeshStatus(const FString& Context)
{
    USkeletalMeshComponent* MeshComp = GetPlayerMeshComponent();
    if (MeshComp)
    {
        USkeletalMesh* CurrentMesh = MeshComp->GetSkeletalMeshAsset();
        FString MeshName = CurrentMesh ? CurrentMesh->GetName() : TEXT("None");
        
        UE_LOG(LogTemp, Log, TEXT("PlayerVisibilityManager [%s]: Mesh = %s, Visible = %s"), 
               *Context, 
               *MeshName,
               MeshComp->IsVisible() ? TEXT("Yes") : TEXT("No"));
    }
}