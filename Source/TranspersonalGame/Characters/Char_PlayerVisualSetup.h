#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Animation/AnimBlueprint.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpringArmComponent.h"
#include "Components/CameraComponent.h"
#include "Char_PlayerVisualSetup.generated.h"

UENUM(BlueprintType)
enum class EChar_PlayerArchetype : uint8
{
    Paleontologist      UMETA(DisplayName = "Paleontologist"),
    TribalHunter        UMETA(DisplayName = "Tribal Hunter"),
    TribalElder         UMETA(DisplayName = "Tribal Elder"),
    TribalScout         UMETA(DisplayName = "Tribal Scout"),
    TribalWarrior       UMETA(DisplayName = "Tribal Warrior")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_PlayerVisualConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    EChar_PlayerArchetype Archetype = EChar_PlayerArchetype::Paleontologist;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    TSoftObjectPtr<USkeletalMesh> PlayerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    TSoftObjectPtr<UAnimBlueprint> AnimationBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Setup")
    float CameraBoomLength = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Setup")
    FRotator CameraBoomRotation = FRotator(-20.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Setup")
    bool bUsePawnControlRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Setup")
    bool bInheritPitch = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Setup")
    bool bInheritYaw = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Setup")
    bool bInheritRoll = false;

    FChar_PlayerVisualConfig()
    {
        // Default to UE5 Mannequin paths
        PlayerMesh = FSoftObjectPath("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple");
        AnimationBlueprint = FSoftObjectPath("/Game/Characters/Mannequins/Animations/ABP_Quinn");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_PlayerVisualSetup : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_PlayerVisualSetup();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Visual Setup")
    FChar_PlayerVisualConfig VisualConfig;

    UFUNCTION(BlueprintCallable, Category = "Player Visual Setup")
    void ApplyVisualConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Player Visual Setup")
    void SetPlayerArchetype(EChar_PlayerArchetype NewArchetype);

    UFUNCTION(BlueprintCallable, Category = "Player Visual Setup")
    void ConfigureCameraSystem();

    UFUNCTION(BlueprintCallable, Category = "Player Visual Setup")
    void SetupPlayerMesh();

    UFUNCTION(BlueprintCallable, Category = "Player Visual Setup")
    void ApplyMaterials();

    UFUNCTION(BlueprintCallable, Category = "Player Visual Setup")
    bool ValidateVisualComponents();

private:
    UPROPERTY()
    USkeletalMeshComponent* PlayerMeshComponent;

    UPROPERTY()
    USpringArmComponent* CameraBoom;

    UPROPERTY()
    UCameraComponent* FollowCamera;

    void InitializeComponents();
    void LoadArchetypeAssets(EChar_PlayerArchetype Archetype);
    FChar_PlayerVisualConfig GetArchetypeConfig(EChar_PlayerArchetype Archetype);
};