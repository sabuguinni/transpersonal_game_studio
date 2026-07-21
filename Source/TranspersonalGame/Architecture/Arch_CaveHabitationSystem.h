#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Arch_CaveHabitationSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_CaveHabitationType : uint8
{
    SimpleShell      UMETA(DisplayName = "Simple Shell"),
    DeepCave         UMETA(DisplayName = "Deep Cave"),
    CaveComplex      UMETA(DisplayName = "Cave Complex"),
    UndergroundChamber UMETA(DisplayName = "Underground Chamber")
};

USTRUCT(BlueprintType)
struct FArch_CaveHabitationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    EArch_CaveHabitationType CaveType = EArch_CaveHabitationType::SimpleShell;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    float CaveDepth = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    float CaveWidth = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    float CaveHeight = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    bool bHasSleepingArea = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    bool bHasToolStorage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    float HumidityLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    float TemperatureModifier = -5.0f;
};

USTRUCT(BlueprintType)
struct FArch_CaveArtifact
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    FString ArtifactName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    TSoftObjectPtr<UStaticMesh> ArtifactMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    bool bIsInteractable = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_CaveHabitationSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_CaveHabitationSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CaveWallsMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CaveFloorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FirePitMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    FArch_CaveHabitationConfig CaveConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Artifacts")
    TArray<FArch_CaveArtifact> CaveArtifacts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* CaveWallMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* CaveFloorMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* FirePitMaterial;

public:
    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void GenerateCaveStructure();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void PlaceCaveArtifacts();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void SetCaveConfiguration(const FArch_CaveHabitationConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    FArch_CaveHabitationConfig GetCaveConfiguration() const { return CaveConfig; }

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void AddArtifact(const FArch_CaveArtifact& NewArtifact);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void RemoveArtifact(int32 ArtifactIndex);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    float GetCaveTemperature() const;

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    float GetCaveHumidity() const { return CaveConfig.HumidityLevel; }

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    bool HasFirePit() const { return CaveConfig.bHasFirePit; }

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    bool HasSleepingArea() const { return CaveConfig.bHasSleepingArea; }

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    bool HasToolStorage() const { return CaveConfig.bHasToolStorage; }

private:
    void CreateCaveGeometry();
    void SetupCaveMaterials();
    void CreateFirePit();
    void CreateSleepingArea();
    void CreateToolStorage();
    void ApplyEnvironmentalEffects();
};