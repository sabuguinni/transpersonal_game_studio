#include "Char_PlayerCharacterVisuals.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UChar_PlayerCharacterVisuals::UChar_PlayerCharacterVisuals()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    // Initialize default visual settings
    VisualSettings.CameraBoomLength = 400.0f;
    VisualSettings.CameraSocketOffset = FVector(0, 0, 60);
    VisualSettings.CameraPitch = -15.0f;
    VisualSettings.bUsePawnControlRotation = true;
    VisualSettings.bInheritPitch = false;
    VisualSettings.bInheritYaw = true;
    VisualSettings.bInheritRoll = false;
    
    // Initialize mesh settings
    MeshSettings.MeshRelativeLocation = FVector(0, 0, -90);
    MeshSettings.MeshRelativeRotation = FRotator(0, -90, 0);
    MeshSettings.MeshRelativeScale = FVector(1.0f, 1.0f, 1.0f);
    
    // Initialize tribal appearance
    TribalAppearance.SkinWeathering = 0.5f;
    TribalAppearance.ClothingWear = 0.3f;
    TribalAppearance.bHasTribalMarkings = true;
    TribalAppearance.bHasBoneAccessories = true;
    
    // Component references
    MeshComponent = nullptr;
    CameraBoom = nullptr;
    FollowCamera = nullptr;
    
    bIsMaleCharacter = true;
    bVisualsInitialized = false;
}

void UChar_PlayerCharacterVisuals::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePlayerVisuals();
}

void UChar_PlayerCharacterVisuals::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UChar_PlayerCharacterVisuals::InitializePlayerVisuals()
{
    if (bVisualsInitialized)
    {
        return;
    }
    
    FindAndCacheComponents();
    
    if (ValidateComponentReferences())
    {
        LoadDefaultMeshes();
        ConfigureMeshComponent();
        SetupCameraSystem();
        ApplyTribalAppearance();
        
        bVisualsInitialized = true;
        
        UE_LOG(LogTemp, Log, TEXT("Player character visuals initialized successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to initialize player visuals - missing components"));
    }
}

void UChar_PlayerCharacterVisuals::SetupCameraSystem()
{
    if (!CameraBoom || !FollowCamera)
    {
        UE_LOG(LogTemp, Warning, TEXT("Camera components not found for visual setup"));
        return;
    }
    
    ApplyCameraSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Camera system configured"));
}

void UChar_PlayerCharacterVisuals::ConfigureMeshComponent()
{
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mesh component not found for configuration"));
        return;
    }
    
    ApplyMeshSettings();
    SetupMaterialOverrides();
    
    UE_LOG(LogTemp, Log, TEXT("Mesh component configured"));
}

void UChar_PlayerCharacterVisuals::ApplyTribalAppearance()
{
    if (!MeshComponent)
    {
        return;
    }
    
    // Apply tribal material overrides
    SetupMaterialOverrides();
    
    UE_LOG(LogTemp, Log, TEXT("Tribal appearance applied"));
}

void UChar_PlayerCharacterVisuals::SetGender(bool bIsMale)
{
    bIsMaleCharacter = bIsMale;
    
    if (bVisualsInitialized && MeshComponent)
    {
        // Load appropriate mesh based on gender
        USkeletalMesh* TargetMesh = nullptr;
        
        if (bIsMale && MeshSettings.MaleMesh.IsValid())
        {
            TargetMesh = MeshSettings.MaleMesh.LoadSynchronous();
        }
        else if (!bIsMale && MeshSettings.FemaleMesh.IsValid())
        {
            TargetMesh = MeshSettings.FemaleMesh.LoadSynchronous();
        }
        else if (MeshSettings.DefaultMesh.IsValid())
        {
            TargetMesh = MeshSettings.DefaultMesh.LoadSynchronous();
        }
        
        if (TargetMesh)
        {
            MeshComponent->SetSkeletalMesh(TargetMesh);
            UE_LOG(LogTemp, Log, TEXT("Gender-specific mesh applied: %s"), bIsMale ? TEXT("Male") : TEXT("Female"));
        }
    }
}

void UChar_PlayerCharacterVisuals::UpdateVisualSettings(const FChar_VisualSettings& NewSettings)
{
    VisualSettings = NewSettings;
    
    if (bVisualsInitialized)
    {
        ApplyCameraSettings();
    }
}

void UChar_PlayerCharacterVisuals::RefreshCharacterAppearance()
{
    if (bVisualsInitialized)
    {
        ConfigureMeshComponent();
        ApplyTribalAppearance();
    }
}

void UChar_PlayerCharacterVisuals::ValidateVisualComponents()
{
    FindAndCacheComponents();
    
    bool bValid = ValidateComponentReferences();
    
    if (bValid)
    {
        UE_LOG(LogTemp, Log, TEXT("All visual components validated successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Visual component validation failed"));
    }
}

void UChar_PlayerCharacterVisuals::FindAndCacheComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Find mesh component
    MeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
    
    // Find camera boom
    CameraBoom = Owner->FindComponentByClass<USpringArmComponent>();
    
    // Find follow camera
    FollowCamera = Owner->FindComponentByClass<UCameraComponent>();
}

void UChar_PlayerCharacterVisuals::LoadDefaultMeshes()
{
    // Try to load common UE5 mannequin meshes
    if (!MeshSettings.DefaultMesh.IsValid())
    {
        // Try various common paths for mannequin meshes
        TArray<FString> PossiblePaths = {
            TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"),
            TEXT("/Engine/Characters/Mannequins/Meshes/SKM_Quinn_Simple"),
            TEXT("/Game/ThirdPerson/Characters/Mannequins/Meshes/SKM_Quinn_Simple"),
            TEXT("/Game/Mannequin/Character/Mesh/SK_Mannequin")
        };
        
        for (const FString& Path : PossiblePaths)
        {
            USkeletalMesh* TestMesh = LoadObject<USkeletalMesh>(nullptr, *Path);
            if (TestMesh)
            {
                MeshSettings.DefaultMesh = TestMesh;
                UE_LOG(LogTemp, Log, TEXT("Loaded default mesh from: %s"), *Path);
                break;
            }
        }
    }
}

void UChar_PlayerCharacterVisuals::ApplyMeshSettings()
{
    if (!MeshComponent)
    {
        return;
    }
    
    // Apply mesh transform
    MeshComponent->SetRelativeLocation(MeshSettings.MeshRelativeLocation);
    MeshComponent->SetRelativeRotation(MeshSettings.MeshRelativeRotation);
    MeshComponent->SetRelativeScale3D(MeshSettings.MeshRelativeScale);
    
    // Apply mesh if available
    USkeletalMesh* TargetMesh = nullptr;
    
    if (bIsMaleCharacter && MeshSettings.MaleMesh.IsValid())
    {
        TargetMesh = MeshSettings.MaleMesh.LoadSynchronous();
    }
    else if (!bIsMaleCharacter && MeshSettings.FemaleMesh.IsValid())
    {
        TargetMesh = MeshSettings.FemaleMesh.LoadSynchronous();
    }
    else if (MeshSettings.DefaultMesh.IsValid())
    {
        TargetMesh = MeshSettings.DefaultMesh.LoadSynchronous();
    }
    
    if (TargetMesh)
    {
        MeshComponent->SetSkeletalMesh(TargetMesh);
    }
}

void UChar_PlayerCharacterVisuals::ApplyCameraSettings()
{
    if (!CameraBoom || !FollowCamera)
    {
        return;
    }
    
    // Configure camera boom
    CameraBoom->TargetArmLength = VisualSettings.CameraBoomLength;
    CameraBoom->SocketOffset = VisualSettings.CameraSocketOffset;
    CameraBoom->bUsePawnControlRotation = VisualSettings.bUsePawnControlRotation;
    CameraBoom->bInheritPitch = VisualSettings.bInheritPitch;
    CameraBoom->bInheritYaw = VisualSettings.bInheritYaw;
    CameraBoom->bInheritRoll = VisualSettings.bInheritRoll;
    
    // Configure camera
    FollowCamera->SetRelativeRotation(FRotator(VisualSettings.CameraPitch, 0, 0));
}

void UChar_PlayerCharacterVisuals::SetupMaterialOverrides()
{
    if (!MeshComponent)
    {
        return;
    }
    
    // Apply skin materials if available
    if (TribalAppearance.SkinMaterials.Num() > 0)
    {
        for (int32 i = 0; i < TribalAppearance.SkinMaterials.Num(); ++i)
        {
            if (TribalAppearance.SkinMaterials[i].IsValid())
            {
                UMaterialInterface* Material = TribalAppearance.SkinMaterials[i].LoadSynchronous();
                if (Material)
                {
                    MeshComponent->SetMaterial(i, Material);
                }
            }
        }
    }
}

bool UChar_PlayerCharacterVisuals::ValidateComponentReferences()
{
    bool bValid = true;
    
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mesh component not found"));
        bValid = false;
    }
    
    if (!CameraBoom)
    {
        UE_LOG(LogTemp, Warning, TEXT("Camera boom not found"));
        bValid = false;
    }
    
    if (!FollowCamera)
    {
        UE_LOG(LogTemp, Warning, TEXT("Follow camera not found"));
        bValid = false;
    }
    
    return bValid;
}