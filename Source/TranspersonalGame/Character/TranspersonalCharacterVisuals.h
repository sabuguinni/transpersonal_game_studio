#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "TranspersonalCharacterVisuals.generated.h"

// ============================================================
// Enums — global scope (UHT requirement)
// ============================================================

/** Visual body type variants for the prehistoric survivor character */
UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Athletic    UMETA(DisplayName = "Athletic"),
    Stocky      UMETA(DisplayName = "Stocky"),
    Lean        UMETA(DisplayName = "Lean"),
    Count       UMETA(Hidden)
};

/** Clothing/equipment tier based on survival progression */
UENUM(BlueprintType)
enum class EChar_ClothingTier : uint8
{
    Naked       UMETA(DisplayName = "Naked Survivor"),
    Primitive   UMETA(DisplayName = "Primitive Hide"),
    Crafted     UMETA(DisplayName = "Crafted Leather"),
    Advanced    UMETA(DisplayName = "Advanced Bone Armor"),
    Count       UMETA(Hidden)
};

/** Dirt/blood/injury overlay state */
UENUM(BlueprintType)
enum class EChar_DirtState : uint8
{
    Clean       UMETA(DisplayName = "Clean"),
    Dusty       UMETA(DisplayName = "Dusty"),
    Muddy       UMETA(DisplayName = "Muddy"),
    Bloodied    UMETA(DisplayName = "Bloodied"),
    Count       UMETA(Hidden)
};

// ============================================================
// Structs — global scope
// ============================================================

/** Visual customization parameters for the player character */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_VisualParams
{
    GENERATED_BODY()

    /** Body type variant */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Visuals")
    EChar_BodyType BodyType = EChar_BodyType::Athletic;

    /** Current clothing/equipment tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Visuals")
    EChar_ClothingTier ClothingTier = EChar_ClothingTier::Primitive;

    /** Dirt/injury state for material blending */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Visuals")
    EChar_DirtState DirtState = EChar_DirtState::Clean;

    /** Skin tone scalar (0.0 = lightest, 1.0 = darkest) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Visuals", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SkinTone = 0.6f;

    /** Muscle definition scalar (0.0 = lean, 1.0 = very muscular) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Visuals", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MuscleDefinition = 0.5f;

    /** Scar coverage scalar (0.0 = none, 1.0 = heavily scarred) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Visuals", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScarCoverage = 0.2f;

    /** Height scale multiplier (0.9 = shorter, 1.1 = taller) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Visuals", meta = (ClampMin = "0.85", ClampMax = "1.15"))
    float HeightScale = 1.0f;
};

/** Bone decoration item worn by the character */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_BoneDecoration
{
    GENERATED_BODY()

    /** Bone socket name to attach to */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Decorations")
    FName SocketName = NAME_None;

    /** Static mesh asset for the decoration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Decorations")
    TSoftObjectPtr<UStaticMesh> DecorationMesh;

    /** Scale of the decoration relative to the socket */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Decorations")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);
};

// ============================================================
// Component
// ============================================================

/**
 * UTranspersonalCharacterVisuals
 * 
 * Actor component that manages the visual appearance of the prehistoric
 * survivor player character. Handles mesh swapping, material parameter
 * updates (skin tone, dirt, blood), and bone decoration attachments.
 * 
 * Attach to: ATranspersonalCharacter
 * Dependencies: USkeletalMeshComponent on owner
 */
UCLASS(ClassGroup = "TranspersonalGame", meta = (BlueprintSpawnableComponent),
       DisplayName = "Character Visuals Component")
class TRANSPERSONALGAME_API UTranspersonalCharacterVisuals : public UActorComponent
{
    GENERATED_BODY()

public:
    UTranspersonalCharacterVisuals();

    // --------------------------------------------------------
    // Visual Parameters
    // --------------------------------------------------------

    /** Current visual customization state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Visuals")
    FChar_VisualParams VisualParams;

    /** Bone decorations worn by this character */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Decorations")
    TArray<FChar_BoneDecoration> BoneDecorations;

    // --------------------------------------------------------
    // Mesh References
    // --------------------------------------------------------

    /** Athletic body skeletal mesh */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Meshes")
    TSoftObjectPtr<USkeletalMesh> AthleticBodyMesh;

    /** Stocky body skeletal mesh */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Meshes")
    TSoftObjectPtr<USkeletalMesh> StockyBodyMesh;

    /** Lean body skeletal mesh */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Meshes")
    TSoftObjectPtr<USkeletalMesh> LeanBodyMesh;

    // --------------------------------------------------------
    // Material Parameter Names
    // --------------------------------------------------------

    /** Material parameter name for skin tone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Materials")
    FName SkinToneParamName = FName("SkinTone");

    /** Material parameter name for dirt overlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Materials")
    FName DirtAmountParamName = FName("DirtAmount");

    /** Material parameter name for blood overlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Materials")
    FName BloodAmountParamName = FName("BloodAmount");

    /** Material parameter name for muscle definition */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Materials")
    FName MuscleParamName = FName("MuscleDefinition");

    /** Material parameter name for scar coverage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Materials")
    FName ScarParamName = FName("ScarCoverage");

    // --------------------------------------------------------
    // Camera Boom Settings (communicated to SpringArmComponent)
    // --------------------------------------------------------

    /** Desired camera boom length for third-person view */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera",
              meta = (ClampMin = "100.0", ClampMax = "1500.0"))
    float CameraBoomLength = 400.0f;

    /** Camera socket offset (up/down adjustment) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera")
    FVector CameraSocketOffset = FVector(0.0f, 0.0f, 80.0f);

    /** Camera pitch angle (negative = looking slightly down) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera",
              meta = (ClampMin = "-60.0", ClampMax = "0.0"))
    float CameraDefaultPitch = -15.0f;

    // --------------------------------------------------------
    // Public API
    // --------------------------------------------------------

    /** Apply current VisualParams to the owner's skeletal mesh */
    UFUNCTION(BlueprintCallable, Category = "Character|Visuals")
    void ApplyVisualParams();

    /** Set skin tone and update material immediately */
    UFUNCTION(BlueprintCallable, Category = "Character|Visuals")
    void SetSkinTone(float NewTone);

    /** Set dirt state and update material immediately */
    UFUNCTION(BlueprintCallable, Category = "Character|Visuals")
    void SetDirtState(EChar_DirtState NewState);

    /** Set clothing tier and swap mesh if needed */
    UFUNCTION(BlueprintCallable, Category = "Character|Visuals")
    void SetClothingTier(EChar_ClothingTier NewTier);

    /** Add blood splatter (0.0-1.0 intensity) */
    UFUNCTION(BlueprintCallable, Category = "Character|Visuals")
    void AddBloodSplatter(float Intensity);

    /** Clear all blood and dirt (e.g. after swimming) */
    UFUNCTION(BlueprintCallable, Category = "Character|Visuals")
    void CleanCharacter();

    /** Attach all BoneDecorations to their sockets */
    UFUNCTION(BlueprintCallable, Category = "Character|Decorations")
    void RefreshBoneDecorations();

    /** Get the correct mesh for the current body type */
    UFUNCTION(BlueprintPure, Category = "Character|Meshes")
    USkeletalMesh* GetMeshForBodyType(EChar_BodyType BodyType) const;

protected:
    virtual void BeginPlay() override;
    virtual void OnRegister() override;

private:
    /** Cached reference to owner's skeletal mesh component */
    UPROPERTY()
    USkeletalMeshComponent* OwnerMeshComponent = nullptr;

    /** Current blood amount (accumulated, clamped 0-1) */
    float CurrentBloodAmount = 0.0f;

    /** Find and cache the owner's skeletal mesh component */
    void CacheOwnerMesh();

    /** Update a scalar material parameter on all mesh materials */
    void SetMaterialScalarParam(FName ParamName, float Value);

    /** Convert EChar_DirtState to a scalar value for the material */
    static float DirtStateToScalar(EChar_DirtState State);
};
