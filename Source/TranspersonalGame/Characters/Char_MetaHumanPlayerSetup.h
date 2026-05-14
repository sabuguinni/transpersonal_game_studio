#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "SharedTypes.h"
#include "Char_MetaHumanPlayerSetup.generated.h"

USTRUCT(BlueprintType)
struct FChar_MetaHumanPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<USkeletalMesh> BodyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<USkeletalMesh> FaceMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<UMaterialInterface> HairMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FVector MeshScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    bool bIsMale;

    FChar_MetaHumanPreset()
    {
        PresetName = TEXT("Default");
        MeshScale = FVector(1.0f);
        bIsMale = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_MetaHumanPlayerSetup : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_MetaHumanPlayerSetup();

protected:
    virtual void BeginPlay() override;

public:
    // Core setup functions
    UFUNCTION(BlueprintCallable, Category = "MetaHuman Setup")
    void SetupPlayerCharacterVisuals();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman Setup")
    void ApplyMetaHumanPreset(const FChar_MetaHumanPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman Setup")
    void SetupThirdPersonCamera();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman Setup")
    void ConfigureCharacterMesh();

    // Preset management
    UFUNCTION(BlueprintCallable, Category = "MetaHuman Setup")
    FChar_MetaHumanPreset GetTribalMalePreset();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman Setup")
    FChar_MetaHumanPreset GetTribalFemalePreset();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman Setup")
    void LoadDefaultMannequinMesh();

    // Camera configuration
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraHeight(float Height);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraAngle(float Pitch);

protected:
    // MetaHuman presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Presets")
    TArray<FChar_MetaHumanPreset> AvailablePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Presets")
    FChar_MetaHumanPreset CurrentPreset;

    // Camera settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    float DefaultCameraDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    float DefaultCameraHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    float DefaultCameraPitch;

    // Component references
    UPROPERTY()
    USkeletalMeshComponent* CharacterMeshComponent;

    UPROPERTY()
    USpringArmComponent* CameraBoomComponent;

    UPROPERTY()
    UCameraComponent* FollowCameraComponent;

private:
    void InitializeDefaultPresets();
    void ValidateComponentReferences();
    bool LoadMetaHumanAsset(const TSoftObjectPtr<UObject>& AssetPath, UObject*& OutAsset);
};