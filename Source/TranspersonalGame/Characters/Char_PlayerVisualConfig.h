#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Animation/AnimBlueprint.h"
#include "Char_PlayerVisualConfig.generated.h"

/**
 * Configuration component for player character visual appearance
 * Handles mesh assignment, materials, and visual customization
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_PlayerMeshConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimBlueprint> AnimationBlueprint;

    FChar_PlayerMeshConfig()
    {
        SkeletalMesh = nullptr;
        Materials.Empty();
        AnimationBlueprint = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CameraConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "100.0", ClampMax = "800.0"))
    float TargetArmLength = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FVector SocketOffset = FVector(0.0f, 0.0f, 80.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bUsePawnControlRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bDoCollisionTest = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "60.0", ClampMax = "120.0"))
    float FieldOfView = 90.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_PlayerVisualConfig : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_PlayerVisualConfig();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Player mesh configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Visual")
    FChar_PlayerMeshConfig PlayerMeshConfig;

    // Camera configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FChar_CameraConfig CameraConfig;

    // Character customization options
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TMap<FString, float> MorphTargetValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TMap<FString, FLinearColor> MaterialColorOverrides;

    // Apply visual configuration to character
    UFUNCTION(BlueprintCallable, Category = "Player Visual")
    void ApplyVisualConfiguration();

    // Load default UE5 mannequin setup
    UFUNCTION(BlueprintCallable, Category = "Player Visual")
    void LoadDefaultMannequinSetup();

    // Configure camera settings
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ApplyCameraConfiguration();

    // Update material parameters
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void UpdateMaterialParameters();

    // Validate mesh and materials are loaded
    UFUNCTION(BlueprintCallable, Category = "Player Visual")
    bool ValidateVisualAssets();

private:
    // Cache references to components
    UPROPERTY()
    class USkeletalMeshComponent* CachedMeshComponent;

    UPROPERTY()
    class USpringArmComponent* CachedCameraBoom;

    UPROPERTY()
    class UCameraComponent* CachedCamera;

    // Initialize component references
    void InitializeComponentReferences();

    // Load assets asynchronously
    void LoadVisualAssets();
};