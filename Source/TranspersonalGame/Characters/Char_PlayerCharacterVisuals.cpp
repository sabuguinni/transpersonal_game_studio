#include "Char_PlayerCharacterVisuals.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"

UChar_PlayerCharacterVisuals::UChar_PlayerCharacterVisuals()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    // Initialize default appearance
    CurrentAppearance.Gender = EChar_CharacterGender::Male;
    CurrentAppearance.SkinTone = EChar_SkinTone::Medium;
    CurrentAppearance.HairStyle = EChar_HairStyle::Long;
    CurrentAppearance.HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f); // Dark brown
    CurrentAppearance.BodyMass = 1.0f;
    CurrentAppearance.Height = 1.0f;

    // Initialize camera settings
    CameraBoomLength = 300.0f;
    CameraBoomRotation = FRotator(-20.0f, 0.0f, 0.0f);
    CameraFOV = 90.0f;
    bUsePawnControlRotation = true;

    LoadDefaultAssets();
}

void UChar_PlayerCharacterVisuals::BeginPlay()
{
    Super::BeginPlay();
    
    SetupCameraComponents();
    RefreshCharacterVisuals();
}

void UChar_PlayerCharacterVisuals::LoadDefaultAssets()
{
    // Try to load UE5 mannequin meshes
    MaleMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple")));
    if (!MaleMesh.IsValid())
    {
        MaleMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Engine/Characters/Mannequins/Meshes/SKM_Quinn_Simple")));
    }

    FemaleMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple")));
    if (!FemaleMesh.IsValid())
    {
        FemaleMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Engine/Characters/Mannequins/Meshes/SKM_Quinn_Simple")));
    }

    // Initialize material arrays with placeholders
    SkinMaterials.Empty();
    SkinMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Engine/BasicShapes/BasicShapeMaterial"))));
    
    HairMaterials.Empty();
    HairMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Engine/BasicShapes/BasicShapeMaterial"))));
}

void UChar_PlayerCharacterVisuals::SetupCameraComponents()
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        return;
    }

    // Find or create camera boom
    USpringArmComponent* CameraBoom = OwnerPawn->FindComponentByClass<USpringArmComponent>();
    if (!CameraBoom)
    {
        CameraBoom = NewObject<USpringArmComponent>(OwnerPawn, TEXT("CameraBoom"));
        if (CameraBoom)
        {
            OwnerPawn->AddInstanceComponent(CameraBoom);
            CameraBoom->AttachToComponent(OwnerPawn->GetRootComponent(), 
                FAttachmentTransformRules::KeepRelativeTransform);
        }
    }

    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = CameraBoomLength;
        CameraBoom->SetRelativeRotation(CameraBoomRotation);
        CameraBoom->bUsePawnControlRotation = bUsePawnControlRotation;
        CameraBoom->bInheritPitch = true;
        CameraBoom->bInheritYaw = true;
        CameraBoom->bInheritRoll = false;
        CameraBoom->bDoCollisionTest = true;
    }

    // Find or create camera component
    UCameraComponent* Camera = OwnerPawn->FindComponentByClass<UCameraComponent>();
    if (!Camera && CameraBoom)
    {
        Camera = NewObject<UCameraComponent>(OwnerPawn, TEXT("FollowCamera"));
        if (Camera)
        {
            OwnerPawn->AddInstanceComponent(Camera);
            Camera->AttachToComponent(CameraBoom, 
                FAttachmentTransformRules::KeepRelativeTransform, 
                USpringArmComponent::SocketName);
        }
    }

    if (Camera)
    {
        Camera->SetFieldOfView(CameraFOV);
        Camera->bUsePawnControlRotation = false;
    }
}

void UChar_PlayerCharacterVisuals::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UChar_PlayerCharacterVisuals::SetCharacterAppearance(const FChar_CharacterAppearance& NewAppearance)
{
    CurrentAppearance = NewAppearance;
    RefreshCharacterVisuals();
}

void UChar_PlayerCharacterVisuals::ApplyGenderMesh(EChar_CharacterGender Gender)
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        return;
    }

    TSoftObjectPtr<USkeletalMesh> TargetMesh = (Gender == EChar_CharacterGender::Male) ? MaleMesh : FemaleMesh;
    
    if (TargetMesh.IsValid())
    {
        USkeletalMesh* LoadedMesh = TargetMesh.LoadSynchronous();
        if (LoadedMesh)
        {
            MeshComp->SetSkeletalMesh(LoadedMesh);
        }
    }

    CurrentAppearance.Gender = Gender;
}

void UChar_PlayerCharacterVisuals::ApplySkinTone(EChar_SkinTone SkinTone)
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        return;
    }

    UMaterialInterface* SkinMaterial = GetSkinMaterialForTone(SkinTone);
    if (SkinMaterial)
    {
        // Apply to all material slots that might be skin
        for (int32 i = 0; i < MeshComp->GetNumMaterials(); ++i)
        {
            MeshComp->SetMaterial(i, SkinMaterial);
        }
    }

    CurrentAppearance.SkinTone = SkinTone;
}

void UChar_PlayerCharacterVisuals::ApplyHairStyle(EChar_HairStyle HairStyle, FLinearColor HairColor)
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        return;
    }

    UMaterialInterface* HairMaterial = GetHairMaterialForStyle(HairStyle);
    if (HairMaterial)
    {
        // Create dynamic material instance to set hair color
        UMaterialInstanceDynamic* DynamicHairMaterial = UMaterialInstanceDynamic::Create(HairMaterial, this);
        if (DynamicHairMaterial)
        {
            DynamicHairMaterial->SetVectorParameterValue(TEXT("HairColor"), HairColor);
            // Apply to hair material slots (assuming slot 1 is hair)
            if (MeshComp->GetNumMaterials() > 1)
            {
                MeshComp->SetMaterial(1, DynamicHairMaterial);
            }
        }
    }

    CurrentAppearance.HairStyle = HairStyle;
    CurrentAppearance.HairColor = HairColor;
}

void UChar_PlayerCharacterVisuals::SetBodyScale(float BodyMass, float Height)
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        return;
    }

    // Apply scaling based on body mass and height
    FVector NewScale = FVector(
        FMath::Lerp(0.8f, 1.2f, BodyMass),  // Width scaling
        FMath::Lerp(0.8f, 1.2f, BodyMass),  // Depth scaling  
        FMath::Lerp(0.9f, 1.1f, Height)     // Height scaling
    );

    MeshComp->SetWorldScale3D(NewScale);

    CurrentAppearance.BodyMass = BodyMass;
    CurrentAppearance.Height = Height;
}

void UChar_PlayerCharacterVisuals::ConfigureThirdPersonCamera()
{
    SetupCameraComponents();
}

void UChar_PlayerCharacterVisuals::SetCameraBoomSettings(float BoomLength, FRotator BoomRotation)
{
    CameraBoomLength = BoomLength;
    CameraBoomRotation = BoomRotation;

    USpringArmComponent* CameraBoom = GetCameraBoom();
    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = CameraBoomLength;
        CameraBoom->SetRelativeRotation(CameraBoomRotation);
    }
}

void UChar_PlayerCharacterVisuals::SetCameraFOV(float NewFOV)
{
    CameraFOV = NewFOV;

    UCameraComponent* Camera = GetCameraComponent();
    if (Camera)
    {
        Camera->SetFieldOfView(CameraFOV);
    }
}

void UChar_PlayerCharacterVisuals::RefreshCharacterVisuals()
{
    ApplyGenderMesh(CurrentAppearance.Gender);
    ApplySkinTone(CurrentAppearance.SkinTone);
    ApplyHairStyle(CurrentAppearance.HairStyle, CurrentAppearance.HairColor);
    SetBodyScale(CurrentAppearance.BodyMass, CurrentAppearance.Height);
}

USkeletalMeshComponent* UChar_PlayerCharacterVisuals::GetCharacterMesh() const
{
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        return OwnerPawn->FindComponentByClass<USkeletalMeshComponent>();
    }
    return nullptr;
}

UCameraComponent* UChar_PlayerCharacterVisuals::GetCameraComponent() const
{
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        return OwnerPawn->FindComponentByClass<UCameraComponent>();
    }
    return nullptr;
}

USpringArmComponent* UChar_PlayerCharacterVisuals::GetCameraBoom() const
{
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        return OwnerPawn->FindComponentByClass<USpringArmComponent>();
    }
    return nullptr;
}

UMaterialInterface* UChar_PlayerCharacterVisuals::GetSkinMaterialForTone(EChar_SkinTone SkinTone) const
{
    int32 MaterialIndex = static_cast<int32>(SkinTone);
    if (SkinMaterials.IsValidIndex(MaterialIndex) && SkinMaterials[MaterialIndex].IsValid())
    {
        return SkinMaterials[MaterialIndex].LoadSynchronous();
    }

    // Fallback to first available material
    if (SkinMaterials.Num() > 0 && SkinMaterials[0].IsValid())
    {
        return SkinMaterials[0].LoadSynchronous();
    }

    return nullptr;
}

UMaterialInterface* UChar_PlayerCharacterVisuals::GetHairMaterialForStyle(EChar_HairStyle HairStyle) const
{
    int32 MaterialIndex = static_cast<int32>(HairStyle);
    if (HairMaterials.IsValidIndex(MaterialIndex) && HairMaterials[MaterialIndex].IsValid())
    {
        return HairMaterials[MaterialIndex].LoadSynchronous();
    }

    // Fallback to first available material
    if (HairMaterials.Num() > 0 && HairMaterials[0].IsValid())
    {
        return HairMaterials[0].LoadSynchronous();
    }

    return nullptr;
}