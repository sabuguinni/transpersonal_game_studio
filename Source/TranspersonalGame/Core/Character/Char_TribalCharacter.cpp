#include "Char_TribalCharacter.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"

AChar_TribalCharacter::AChar_TribalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Configure capsule collision
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
    GetCapsuleComponent()->SetCapsuleRadius(34.0f);

    // Configure character mesh
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Create spring arm component for camera
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 400.0f;
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bInheritPitch = true;
    SpringArm->bInheritYaw = true;
    SpringArm->bInheritRoll = false;
    SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

    // Create follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Create clothing mesh components
    ChestClothingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestClothing"));
    ChestClothingMesh->SetupAttachment(GetMesh(), TEXT("spine_03"));
    ChestClothingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    LegClothingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LegClothing"));
    LegClothingMesh->SetupAttachment(GetMesh(), TEXT("pelvis"));
    LegClothingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FootClothingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FootClothing"));
    FootClothingMesh->SetupAttachment(GetMesh(), TEXT("foot_l"));
    FootClothingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    HeadAccessoryMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadAccessory"));
    HeadAccessoryMesh->SetupAttachment(GetMesh(), TEXT("head"));
    HeadAccessoryMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Configure movement
    ConfigureMovementSettings();

    // Set default appearance
    SetupDefaultAppearance();
}

void AChar_TribalCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCharacterComponents();
    SetupDefaultMannequinMesh();
    ApplyAppearanceToMesh();
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("AChar_TribalCharacter initialized: %s"), *CharacterName);
}

void AChar_TribalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update appearance periodically if needed
    if (bIsInitialized && GetWorld()->GetTimeSeconds() - LastAppearanceUpdateTime > 1.0f)
    {
        LastAppearanceUpdateTime = GetWorld()->GetTimeSeconds();
        // Periodic updates can be added here
    }
}

void AChar_TribalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    // Basic movement bindings will be handled by the game mode
    // This character class focuses on appearance and tribal characteristics
}

void AChar_TribalCharacter::SetCharacterGender(EChar_TribalGender NewGender)
{
    Gender = NewGender;
    
    // Adjust appearance based on gender
    switch (Gender)
    {
        case EChar_TribalGender::Male:
            Appearance.Height = FMath::RandRange(0.95f, 1.1f);
            Appearance.BodyMassIndex = FMath::RandRange(0.9f, 1.2f);
            break;
        case EChar_TribalGender::Female:
            Appearance.Height = FMath::RandRange(0.85f, 1.0f);
            Appearance.BodyMassIndex = FMath::RandRange(0.8f, 1.1f);
            break;
        case EChar_TribalGender::Child:
            Appearance.Height = FMath::RandRange(0.6f, 0.8f);
            Appearance.BodyMassIndex = FMath::RandRange(0.7f, 0.9f);
            Appearance.Age = FMath::RandRange(8, 16);
            break;
    }
    
    ApplyAppearanceToMesh();
}

void AChar_TribalCharacter::SetTribalRole(EChar_TribalRole NewRole)
{
    TribalRole = NewRole;
    
    // Adjust appearance and clothing based on role
    switch (TribalRole)
    {
        case EChar_TribalRole::Hunter:
            Appearance.BodyMassIndex = FMath::RandRange(0.9f, 1.1f);
            Appearance.bHasScars = FMath::RandBool();
            Clothing.ClothingColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f); // Dark brown
            break;
        case EChar_TribalRole::Gatherer:
            Appearance.BodyMassIndex = FMath::RandRange(0.8f, 1.0f);
            Clothing.ClothingColor = FLinearColor(0.6f, 0.5f, 0.3f, 1.0f); // Light brown
            break;
        case EChar_TribalRole::Shaman:
            Appearance.bHasTattoos = true;
            Appearance.Age = FMath::RandRange(40, 70);
            Clothing.ClothingColor = FLinearColor(0.3f, 0.2f, 0.4f, 1.0f); // Purple-brown
            break;
        case EChar_TribalRole::Crafter:
            Appearance.BodyMassIndex = FMath::RandRange(0.85f, 1.05f);
            Clothing.ClothingColor = FLinearColor(0.5f, 0.4f, 0.3f, 1.0f); // Medium brown
            break;
        case EChar_TribalRole::Scout:
            Appearance.Height = FMath::RandRange(0.9f, 1.05f);
            Appearance.BodyMassIndex = FMath::RandRange(0.8f, 0.95f);
            Clothing.ClothingColor = FLinearColor(0.3f, 0.4f, 0.2f, 1.0f); // Green-brown
            break;
        case EChar_TribalRole::Elder:
            Appearance.Age = FMath::RandRange(50, 80);
            Appearance.bHasTattoos = true;
            Clothing.ClothingColor = FLinearColor(0.7f, 0.6f, 0.4f, 1.0f); // Light tan
            break;
    }
    
    UpdateClothingMeshes();
}

void AChar_TribalCharacter::SetCharacterAppearance(const FChar_TribalAppearance& NewAppearance)
{
    Appearance = NewAppearance;
    ApplyAppearanceToMesh();
}

void AChar_TribalCharacter::SetCharacterClothing(const FChar_TribalClothing& NewClothing)
{
    Clothing = NewClothing;
    UpdateClothingMeshes();
}

void AChar_TribalCharacter::ApplyRandomTribalAppearance()
{
    // Generate random appearance
    Appearance.SkinTone = GenerateRandomSkinTone();
    Appearance.HairColor = GenerateRandomHairColor();
    Appearance.Height = FMath::RandRange(0.85f, 1.15f);
    Appearance.BodyMassIndex = FMath::RandRange(0.8f, 1.2f);
    Appearance.Age = FMath::RandRange(18, 60);
    Appearance.bHasScars = FMath::RandRange(0.0f, 1.0f) > 0.7f;
    Appearance.bHasTattoos = FMath::RandRange(0.0f, 1.0f) > 0.6f;
    
    // Generate random clothing color
    Clothing.ClothingColor = FLinearColor(
        FMath::RandRange(0.2f, 0.8f),
        FMath::RandRange(0.2f, 0.6f),
        FMath::RandRange(0.1f, 0.4f),
        1.0f
    );
    
    ApplyAppearanceToMesh();
    UpdateClothingMeshes();
}

void AChar_TribalCharacter::SetupDefaultMannequinMesh()
{
    // Try to load UE5 default mannequin mesh
    USkeletalMesh* MannequinMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Engine/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
    
    if (!MannequinMesh)
    {
        // Fallback to other potential mannequin paths
        MannequinMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn"));
    }
    
    if (!MannequinMesh)
    {
        // Try the classic UE4 mannequin
        MannequinMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Engine/Characters/Mannequins/Meshes/SK_Mannequin"));
    }
    
    if (MannequinMesh)
    {
        GetMesh()->SetSkeletalMesh(MannequinMesh);
        UE_LOG(LogTemp, Warning, TEXT("Tribal character mesh set successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load mannequin mesh for tribal character"));
    }
}

void AChar_TribalCharacter::ConfigureCameraSystem(float ArmLength, float ArmHeight)
{
    if (SpringArm)
    {
        SpringArm->TargetArmLength = ArmLength;
        SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, ArmHeight));
        SpringArm->bDoCollisionTest = true;
        SpringArm->bUsePawnControlRotation = true;
    }
}

void AChar_TribalCharacter::UpdateSkinMaterial()
{
    if (!GetMesh() || !GetMesh()->GetSkeletalMeshAsset())
    {
        return;
    }
    
    // Create dynamic material instance for skin
    UMaterialInterface* BaseMaterial = GetMesh()->GetMaterial(0);
    if (BaseMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("SkinColor"), Appearance.SkinTone);
            DynamicMaterial->SetVectorParameterValue(TEXT("HairColor"), Appearance.HairColor);
            DynamicMaterial->SetScalarParameterValue(TEXT("ScarIntensity"), Appearance.bHasScars ? 1.0f : 0.0f);
            DynamicMaterial->SetScalarParameterValue(TEXT("TattooIntensity"), Appearance.bHasTattoos ? 1.0f : 0.0f);
            
            GetMesh()->SetMaterial(0, DynamicMaterial);
        }
    }
}

void AChar_TribalCharacter::UpdateClothingMeshes()
{
    // Update clothing mesh materials with the clothing color
    TArray<UStaticMeshComponent*> ClothingComponents = {
        ChestClothingMesh, LegClothingMesh, FootClothingMesh, HeadAccessoryMesh
    };
    
    for (UStaticMeshComponent* ClothingComp : ClothingComponents)
    {
        if (ClothingComp && ClothingComp->GetStaticMesh())
        {
            UMaterialInterface* BaseMaterial = ClothingComp->GetMaterial(0);
            if (BaseMaterial)
            {
                UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
                if (DynamicMaterial)
                {
                    DynamicMaterial->SetVectorParameterValue(TEXT("ClothingColor"), Clothing.ClothingColor);
                    ClothingComp->SetMaterial(0, DynamicMaterial);
                }
            }
        }
    }
}

void AChar_TribalCharacter::SetSkeletalMeshFromPath(const FString& MeshPath)
{
    USkeletalMesh* NewMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
    if (NewMesh)
    {
        GetMesh()->SetSkeletalMesh(NewMesh);
        ApplyAppearanceToMesh();
        UE_LOG(LogTemp, Warning, TEXT("Tribal character mesh updated: %s"), *MeshPath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load mesh: %s"), *MeshPath);
    }
}

void AChar_TribalCharacter::InitializeCharacterComponents()
{
    // Ensure all components are properly initialized
    if (SpringArm)
    {
        SpringArm->bDoCollisionTest = true;
        SpringArm->bInheritPitch = true;
        SpringArm->bInheritYaw = true;
        SpringArm->bInheritRoll = false;
    }
    
    if (FollowCamera)
    {
        FollowCamera->bUsePawnControlRotation = false;
    }
    
    // Initialize clothing components
    TArray<UStaticMeshComponent*> ClothingComponents = {
        ChestClothingMesh, LegClothingMesh, FootClothingMesh, HeadAccessoryMesh
    };
    
    for (UStaticMeshComponent* ClothingComp : ClothingComponents)
    {
        if (ClothingComp)
        {
            ClothingComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            ClothingComp->SetCastShadow(true);
        }
    }
}

void AChar_TribalCharacter::SetupDefaultAppearance()
{
    // Set default tribal appearance
    Appearance.SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f); // Warm tan
    Appearance.HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f); // Dark brown
    Appearance.Height = 1.0f;
    Appearance.BodyMassIndex = 1.0f;
    Appearance.Age = 25;
    Appearance.bHasScars = false;
    Appearance.bHasTattoos = false;
    
    // Set default clothing
    Clothing.ClothingColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f); // Leather brown
    
    CharacterName = TEXT("Tribal Survivor");
}

void AChar_TribalCharacter::ApplyAppearanceToMesh()
{
    if (!GetMesh())
    {
        return;
    }
    
    // Apply scale based on height and body mass
    FVector NewScale = FVector(
        Appearance.BodyMassIndex,
        Appearance.BodyMassIndex,
        Appearance.Height
    );
    GetMesh()->SetRelativeScale3D(NewScale);
    
    // Update skin material
    UpdateSkinMaterial();
}

FLinearColor AChar_TribalCharacter::GenerateRandomSkinTone()
{
    // Generate realistic skin tones for prehistoric humans
    float BaseR = FMath::RandRange(0.6f, 0.9f);
    float BaseG = FMath::RandRange(0.4f, 0.7f);
    float BaseB = FMath::RandRange(0.3f, 0.6f);
    
    return FLinearColor(BaseR, BaseG, BaseB, 1.0f);
}

FLinearColor AChar_TribalCharacter::GenerateRandomHairColor()
{
    // Generate natural hair colors
    TArray<FLinearColor> HairColors = {
        FLinearColor(0.1f, 0.05f, 0.02f, 1.0f), // Black
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f),  // Dark brown
        FLinearColor(0.3f, 0.2f, 0.1f, 1.0f),   // Brown
        FLinearColor(0.4f, 0.3f, 0.2f, 1.0f),   // Light brown
        FLinearColor(0.5f, 0.4f, 0.1f, 1.0f)    // Dirty blonde
    };
    
    int32 RandomIndex = FMath::RandRange(0, HairColors.Num() - 1);
    return HairColors[RandomIndex];
}

void AChar_TribalCharacter::ConfigureMovementSettings()
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        // Configure realistic prehistoric human movement
        Movement->MaxWalkSpeed = 400.0f;
        Movement->MaxAcceleration = 1024.0f;
        Movement->BrakingDecelerationWalking = 1024.0f;
        Movement->GroundFriction = 8.0f;
        Movement->JumpZVelocity = 420.0f;
        Movement->AirControl = 0.2f;
        Movement->bCanWalkOffLedges = true;
        Movement->bCanWalkOffLedgesWhenCrouching = true;
    }
}