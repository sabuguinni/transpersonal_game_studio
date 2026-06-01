#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "../SharedTypes.h"
#include "Char_PlayerVisibilityManager.generated.h"

/**
 * Character Artist Agent #9 - Player Visibility Manager
 * Ensures the player character has a visible mesh and proper visual setup
 * Handles fallback meshes when custom assets are not available
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_PlayerVisibilityManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_PlayerVisibilityManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Mesh Management
    UFUNCTION(BlueprintCallable, Category = "Player Visibility")
    void SetupPlayerMesh();

    UFUNCTION(BlueprintCallable, Category = "Player Visibility")
    void ApplyFallbackMesh();

    UFUNCTION(BlueprintCallable, Category = "Player Visibility")
    void SetCustomMesh(USkeletalMesh* NewMesh);

    UFUNCTION(BlueprintCallable, Category = "Player Visibility")
    bool ValidateMeshVisibility();

    // Camera Setup
    UFUNCTION(BlueprintCallable, Category = "Player Visibility")
    void ConfigureThirdPersonCamera();

    UFUNCTION(BlueprintCallable, Category = "Player Visibility")
    void AdjustCameraBoom(float ArmLength, FVector SocketOffset);

    // Material Management
    UFUNCTION(BlueprintCallable, Category = "Player Visibility")
    void ApplyBaseMaterials();

    UFUNCTION(BlueprintCallable, Category = "Player Visibility")
    void SetSkinMaterial(UMaterialInterface* SkinMaterial);

    UFUNCTION(BlueprintCallable, Category = "Player Visibility")
    void SetClothingMaterial(UMaterialInterface* ClothingMaterial);

protected:
    // Mesh References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Setup")
    TSoftObjectPtr<USkeletalMesh> PreferredPlayerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Setup")
    TSoftObjectPtr<USkeletalMesh> FallbackMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Setup")
    TArray<TSoftObjectPtr<USkeletalMesh>> AvailableMeshes;

    // Material References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> DefaultSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> DefaultClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinVariations;

    // Camera Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float DefaultCameraArmLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FVector DefaultCameraOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FRotator DefaultCameraRotation;

    // Visibility Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visibility")
    bool bEnsureVisibleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visibility")
    bool bAutoApplyFallback;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visibility")
    float MeshValidationInterval;

private:
    // Internal state
    UPROPERTY()
    USkeletalMeshComponent* CachedMeshComponent;

    UPROPERTY()
    class USpringArmComponent* CachedCameraBoom;

    UPROPERTY()
    class UCameraComponent* CachedCamera;

    float LastValidationTime;
    bool bMeshSetupComplete;

    // Helper functions
    USkeletalMeshComponent* GetPlayerMeshComponent();
    class USpringArmComponent* GetCameraBoom();
    class UCameraComponent* GetCamera();
    
    void LoadFallbackMeshes();
    bool TryLoadMesh(const TSoftObjectPtr<USkeletalMesh>& MeshPtr);
    void LogMeshStatus(const FString& Context);
};