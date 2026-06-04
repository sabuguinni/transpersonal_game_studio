#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpringArmComponent.h"
#include "Components/CameraComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Char_PlayerCharacterVisuals.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_VisualSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    float CameraBoomLength = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    FVector CameraSocketOffset = FVector(0, 0, 60);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    float CameraPitch = -15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    bool bUsePawnControlRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    bool bInheritPitch = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    bool bInheritYaw = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    bool bInheritRoll = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_MeshSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Settings")
    TSoftObjectPtr<USkeletalMesh> DefaultMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Settings")
    TSoftObjectPtr<USkeletalMesh> MaleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Settings")
    TSoftObjectPtr<USkeletalMesh> FemaleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Settings")
    FVector MeshRelativeLocation = FVector(0, 0, -90);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Settings")
    FRotator MeshRelativeRotation = FRotator(0, -90, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Settings")
    FVector MeshRelativeScale = FVector(1.0f, 1.0f, 1.0f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    TArray<TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    TArray<TSoftObjectPtr<UMaterialInterface>> HairMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    float SkinWeathering = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    float ClothingWear = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    bool bHasTribalMarkings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    bool bHasBoneAccessories = true;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_PlayerCharacterVisuals : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_PlayerCharacterVisuals();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void InitializePlayerVisuals();

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetupCameraSystem();

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void ConfigureMeshComponent();

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void ApplyTribalAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetGender(bool bIsMale);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void UpdateVisualSettings(const FChar_VisualSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void RefreshCharacterAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Visuals", CallInEditor)
    void ValidateVisualComponents();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    FChar_VisualSettings VisualSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Settings")
    FChar_MeshSettings MeshSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    FChar_TribalAppearance TribalAppearance;

    UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USkeletalMeshComponent> MeshComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character State")
    bool bIsMaleCharacter = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character State")
    bool bVisualsInitialized = false;

private:
    void FindAndCacheComponents();
    void LoadDefaultMeshes();
    void ApplyMeshSettings();
    void ApplyCameraSettings();
    void SetupMaterialOverrides();
    bool ValidateComponentReferences();
};