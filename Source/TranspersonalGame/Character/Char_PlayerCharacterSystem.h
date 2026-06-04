#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TranspersonalGame/TranspersonalGame.h"
#include "Char_PlayerCharacterSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_PlayerAppearance : uint8
{
    TribalMale UMETA(DisplayName = "Tribal Male"),
    TribalFemale UMETA(DisplayName = "Tribal Female"),
    HunterMale UMETA(DisplayName = "Hunter Male"),
    HunterFemale UMETA(DisplayName = "Hunter Female"),
    ElderMale UMETA(DisplayName = "Elder Male"),
    ElderFemale UMETA(DisplayName = "Elder Female"),
    ShamanMale UMETA(DisplayName = "Shaman Male"),
    ShamanFemale UMETA(DisplayName = "Shaman Female")
};

UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    AnimalHide UMETA(DisplayName = "Animal Hide"),
    LeatherWraps UMETA(DisplayName = "Leather Wraps"),
    FurCloak UMETA(DisplayName = "Fur Cloak"),
    TribalPaint UMETA(DisplayName = "Tribal Paint"),
    BoneOrnaments UMETA(DisplayName = "Bone Ornaments"),
    FeatherDecorations UMETA(DisplayName = "Feather Decorations")
};

USTRUCT(BlueprintType)
struct FChar_CharacterCustomization
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_PlayerAppearance AppearanceType = EChar_PlayerAppearance::TribalMale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingStyle ClothingStyle = EChar_ClothingStyle::AnimalHide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SkinTone = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor ClothingColor = FLinearColor::Brown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTribalMarkings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasBoneJewelry = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasFeathers = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float BodyScale = 1.0f;

    FChar_CharacterCustomization()
    {
        AppearanceType = EChar_PlayerAppearance::TribalMale;
        ClothingStyle = EChar_ClothingStyle::AnimalHide;
        SkinTone = 0.5f;
        HairColor = FLinearColor::Black;
        ClothingColor = FLinearColor(0.4f, 0.2f, 0.1f, 1.0f); // Brown
        bHasTribalMarkings = true;
        bHasBoneJewelry = false;
        bHasFeathers = false;
        BodyScale = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_PlayerCharacterSystem : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_PlayerCharacterSystem();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Camera Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;

    // Character Appearance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_CharacterCustomization CharacterCustomization;

    // Mesh Components for Clothing/Accessories
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
    class UStaticMeshComponent* ClothingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
    class UStaticMeshComponent* AccessoryMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
    class UStaticMeshComponent* WeaponMesh;

    // Materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* BaseSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* HairMaterial;

    // Dynamic Material Instances
    UPROPERTY(BlueprintReadOnly, Category = "Materials")
    class UMaterialInstanceDynamic* DynamicSkinMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Materials")
    class UMaterialInstanceDynamic* DynamicClothingMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Materials")
    class UMaterialInstanceDynamic* DynamicHairMaterial;

    // Movement Input Handlers
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void StartJump();
    void StopJump();
    void StartRun();
    void StopRun();
    void StartCrouch();
    void StopCrouch();

    // Character Customization Functions
    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplyCharacterCustomization(const FChar_CharacterCustomization& NewCustomization);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetAppearanceType(EChar_PlayerAppearance NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetClothingStyle(EChar_ClothingStyle NewClothingStyle);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetSkinTone(float NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetHairColor(FLinearColor NewHairColor);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetClothingColor(FLinearColor NewClothingColor);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetTribalMarkings(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetBoneJewelry(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetFeathers(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetBodyScale(float NewScale);

    // Mesh Setup Functions
    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void SetupCharacterMesh();

    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void SetupCameraSystem();

    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void SetupMovementComponent();

    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void CreateDynamicMaterials();

    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void ApplyMaterialsToMesh();

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Character Appearance")
    FString GetAppearanceDescription() const;

    UFUNCTION(BlueprintPure, Category = "Character Appearance")
    bool IsCharacterFullyCustomized() const;

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void RandomizeAppearance();

    // Camera Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    float CameraBoomLength = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    FRotator CameraBoomRotation = FRotator(-20.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    bool bUsePawnControlRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    bool bInheritPitch = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    bool bInheritYaw = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    bool bInheritRoll = false;

    // Movement Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float RunSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float CrouchSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float JumpVelocity = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float AirControl = 0.2f;

    // State Variables
    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsRunning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsCustomizationApplied = false;

public:
    // Getters
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    FORCEINLINE FChar_CharacterCustomization GetCharacterCustomization() const { return CharacterCustomization; }
};