#include "Char_PlayerVisualController.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlayerVisualController, Log, All);

UChar_PlayerVisualController::UChar_PlayerVisualController()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default values
    bIsInitialized = false;
    CurrentVisualPreset = EChar_VisualPreset::TribalHunter;
    
    // Set default mesh paths - these will be overridden by actual MetaHuman assets
    DefaultMaleMeshPath = TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn");
    DefaultFemaleMeshPath = TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn");
    
    UE_LOG(LogPlayerVisualController, Log, TEXT("PlayerVisualController component created"));
}

void UChar_PlayerVisualController::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the visual system
    InitializePlayerVisuals();
    
    UE_LOG(LogPlayerVisualController, Log, TEXT("PlayerVisualController initialized in BeginPlay"));
}

void UChar_PlayerVisualController::InitializePlayerVisuals()
{
    if (bIsInitialized)
    {
        UE_LOG(LogPlayerVisualController, Warning, TEXT("PlayerVisualController already initialized"));
        return;
    }
    
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogPlayerVisualController, Error, TEXT("PlayerVisualController owner is not a Character"));
        return;
    }
    
    // Get the skeletal mesh component
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        UE_LOG(LogPlayerVisualController, Error, TEXT("Character has no skeletal mesh component"));
        return;
    }
    
    // Set up default character mesh
    SetupDefaultCharacterMesh(MeshComp);
    
    // Configure camera system
    SetupCameraSystem(OwnerCharacter);
    
    // Apply initial visual preset
    ApplyVisualPreset(CurrentVisualPreset);
    
    bIsInitialized = true;
    UE_LOG(LogPlayerVisualController, Log, TEXT("PlayerVisualController initialization complete"));
}

void UChar_PlayerVisualController::SetupDefaultCharacterMesh(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent)
    {
        UE_LOG(LogPlayerVisualController, Error, TEXT("Invalid mesh component for setup"));
        return;
    }
    
    // Try to load default mannequin mesh
    USkeletalMesh* DefaultMesh = LoadObject<USkeletalMesh>(nullptr, *DefaultMaleMeshPath);
    if (DefaultMesh)
    {
        MeshComponent->SetSkeletalMesh(DefaultMesh);
        UE_LOG(LogPlayerVisualController, Log, TEXT("Default character mesh loaded: %s"), *DefaultMaleMeshPath);
    }
    else
    {
        UE_LOG(LogPlayerVisualController, Warning, TEXT("Could not load default mesh: %s"), *DefaultMaleMeshPath);
        
        // Fallback: try to find any available character mesh
        TArray<FString> AvailableMeshes = FindAvailableCharacterMeshes();
        if (AvailableMeshes.Num() > 0)
        {
            USkeletalMesh* FallbackMesh = LoadObject<USkeletalMesh>(nullptr, *AvailableMeshes[0]);
            if (FallbackMesh)
            {
                MeshComponent->SetSkeletalMesh(FallbackMesh);
                UE_LOG(LogPlayerVisualController, Log, TEXT("Fallback mesh loaded: %s"), *AvailableMeshes[0]);
            }
        }
    }
    
    // Configure mesh component settings
    MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
    MeshComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
}

void UChar_PlayerVisualController::SetupCameraSystem(ACharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogPlayerVisualController, Error, TEXT("Invalid character for camera setup"));
        return;
    }
    
    // Find or create spring arm component
    USpringArmComponent* SpringArm = Character->FindComponentByClass<USpringArmComponent>();
    if (!SpringArm)
    {
        SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
        SpringArm->SetupAttachment(Character->GetRootComponent());
        UE_LOG(LogPlayerVisualController, Log, TEXT("Created new SpringArm component"));
    }
    
    if (SpringArm)
    {
        // Configure spring arm for third-person view
        SpringArm->TargetArmLength = 400.0f;
        SpringArm->bUsePawnControlRotation = true;
        SpringArm->bInheritPitch = true;
        SpringArm->bInheritYaw = true;
        SpringArm->bInheritRoll = false;
        SpringArm->bDoCollisionTest = true;
        SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
        
        UE_LOG(LogPlayerVisualController, Log, TEXT("SpringArm configured for third-person view"));
    }
    
    // Find or create camera component
    UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>();
    if (!Camera && SpringArm)
    {
        Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
        Camera->SetupAttachment(SpringArm);
        UE_LOG(LogPlayerVisualController, Log, TEXT("Created new Camera component"));
    }
    
    if (Camera)
    {
        Camera->SetFieldOfView(90.0f);
        UE_LOG(LogPlayerVisualController, Log, TEXT("Camera configured"));
    }
}

TArray<FString> UChar_PlayerVisualController::FindAvailableCharacterMeshes()
{
    TArray<FString> AvailableMeshes;
    
    // Common UE5 character mesh paths to try
    TArray<FString> CommonPaths = {
        TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn"),
        TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny"),
        TEXT("/Engine/Characters/Mannequins/Meshes/SKM_Quinn"),
        TEXT("/Engine/Characters/Mannequins/Meshes/SKM_Manny"),
        TEXT("/Game/ThirdPerson/Meshes/SKM_Quinn"),
        TEXT("/Game/ThirdPersonBP/Meshes/SKM_Quinn")
    };
    
    for (const FString& Path : CommonPaths)
    {
        USkeletalMesh* TestMesh = LoadObject<USkeletalMesh>(nullptr, *Path);
        if (TestMesh)
        {
            AvailableMeshes.Add(Path);
            UE_LOG(LogPlayerVisualController, Log, TEXT("Found available mesh: %s"), *Path);
        }
    }
    
    return AvailableMeshes;
}

void UChar_PlayerVisualController::ApplyVisualPreset(EChar_VisualPreset Preset)
{
    CurrentVisualPreset = Preset;
    
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogPlayerVisualController, Error, TEXT("Cannot apply visual preset - invalid owner"));
        return;
    }
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        UE_LOG(LogPlayerVisualController, Error, TEXT("Cannot apply visual preset - no mesh component"));
        return;
    }
    
    switch (Preset)
    {
        case EChar_VisualPreset::TribalHunter:
            ApplyTribalHunterPreset(MeshComp);
            break;
            
        case EChar_VisualPreset::TribalGatherer:
            ApplyTribalGathererPreset(MeshComp);
            break;
            
        case EChar_VisualPreset::TribalShaman:
            ApplyTribalShamanPreset(MeshComp);
            break;
            
        case EChar_VisualPreset::TribalWarrior:
            ApplyTribalWarriorPreset(MeshComp);
            break;
            
        default:
            UE_LOG(LogPlayerVisualController, Warning, TEXT("Unknown visual preset: %d"), (int32)Preset);
            break;
    }
    
    UE_LOG(LogPlayerVisualController, Log, TEXT("Applied visual preset: %d"), (int32)Preset);
}

void UChar_PlayerVisualController::ApplyTribalHunterPreset(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent)
        return;
        
    // Create dynamic material instance for skin customization
    UMaterialInterface* BaseMaterial = MeshComponent->GetMaterial(0);
    if (BaseMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (DynamicMaterial)
        {
            // Apply hunter-specific material parameters
            DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), 0.7f); // Bronze skin
            DynamicMaterial->SetVectorParameterValue(TEXT("SkinColor"), FLinearColor(0.8f, 0.6f, 0.4f, 1.0f));
            DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), 0.8f); // High weathering
            
            MeshComponent->SetMaterial(0, DynamicMaterial);
        }
    }
    
    UE_LOG(LogPlayerVisualController, Log, TEXT("Applied Tribal Hunter visual preset"));
}

void UChar_PlayerVisualController::ApplyTribalGathererPreset(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent)
        return;
        
    UMaterialInterface* BaseMaterial = MeshComponent->GetMaterial(0);
    if (BaseMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (DynamicMaterial)
        {
            // Apply gatherer-specific material parameters
            DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), 0.5f); // Medium skin
            DynamicMaterial->SetVectorParameterValue(TEXT("SkinColor"), FLinearColor(0.7f, 0.5f, 0.3f, 1.0f));
            DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), 0.4f); // Moderate weathering
            
            MeshComponent->SetMaterial(0, DynamicMaterial);
        }
    }
    
    UE_LOG(LogPlayerVisualController, Log, TEXT("Applied Tribal Gatherer visual preset"));
}

void UChar_PlayerVisualController::ApplyTribalShamanPreset(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent)
        return;
        
    UMaterialInterface* BaseMaterial = MeshComponent->GetMaterial(0);
    if (BaseMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (DynamicMaterial)
        {
            // Apply shaman-specific material parameters
            DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), 0.3f); // Darker skin
            DynamicMaterial->SetVectorParameterValue(TEXT("SkinColor"), FLinearColor(0.5f, 0.4f, 0.3f, 1.0f));
            DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), 0.6f); // Moderate weathering
            DynamicMaterial->SetScalarParameterValue(TEXT("Tattoos"), 1.0f); // Visible tattoos
            
            MeshComponent->SetMaterial(0, DynamicMaterial);
        }
    }
    
    UE_LOG(LogPlayerVisualController, Log, TEXT("Applied Tribal Shaman visual preset"));
}

void UChar_PlayerVisualController::ApplyTribalWarriorPreset(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent)
        return;
        
    UMaterialInterface* BaseMaterial = MeshComponent->GetMaterial(0);
    if (BaseMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (DynamicMaterial)
        {
            // Apply warrior-specific material parameters
            DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), 0.6f); // Medium-dark skin
            DynamicMaterial->SetVectorParameterValue(TEXT("SkinColor"), FLinearColor(0.6f, 0.4f, 0.3f, 1.0f));
            DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), 0.9f); // High weathering
            DynamicMaterial->SetScalarParameterValue(TEXT("Scars"), 0.7f); // Battle scars
            
            MeshComponent->SetMaterial(0, DynamicMaterial);
        }
    }
    
    UE_LOG(LogPlayerVisualController, Log, TEXT("Applied Tribal Warrior visual preset"));
}

void UChar_PlayerVisualController::SetCharacterGender(EChar_Gender NewGender)
{
    CurrentGender = NewGender;
    
    // Update mesh based on gender
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
        if (MeshComp)
        {
            FString MeshPath = (NewGender == EChar_Gender::Male) ? DefaultMaleMeshPath : DefaultFemaleMeshPath;
            USkeletalMesh* NewMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
            
            if (NewMesh)
            {
                MeshComp->SetSkeletalMesh(NewMesh);
                UE_LOG(LogPlayerVisualController, Log, TEXT("Character gender changed to: %s"), 
                    (NewGender == EChar_Gender::Male) ? TEXT("Male") : TEXT("Female"));
            }
        }
    }
}

EChar_Gender UChar_PlayerVisualController::GetCharacterGender() const
{
    return CurrentGender;
}

void UChar_PlayerVisualController::SetVisualPreset(EChar_VisualPreset NewPreset)
{
    if (NewPreset != CurrentVisualPreset)
    {
        ApplyVisualPreset(NewPreset);
    }
}

EChar_VisualPreset UChar_PlayerVisualController::GetVisualPreset() const
{
    return CurrentVisualPreset;
}

bool UChar_PlayerVisualController::IsInitialized() const
{
    return bIsInitialized;
}