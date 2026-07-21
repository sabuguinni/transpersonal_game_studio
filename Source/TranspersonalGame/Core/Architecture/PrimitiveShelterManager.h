#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "PrimitiveShelterManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Config")
    ESurvivalBiome BiomeType = ESurvivalBiome::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Config")
    int32 WallCount = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Config")
    float ShelterRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Config")
    float WallHeight = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Config")
    bool bHasRoof = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Config")
    bool bHasFirepit = true;

    FArch_ShelterConfiguration()
    {
        BiomeType = ESurvivalBiome::Savana;
        WallCount = 4;
        ShelterRadius = 300.0f;
        WallHeight = 200.0f;
        bHasRoof = true;
        bHasFirepit = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterComponent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    UStaticMesh* ComponentMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    UMaterialInterface* ComponentMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    FVector Scale = FVector::OneVector;

    FArch_ShelterComponent()
    {
        ComponentMesh = nullptr;
        ComponentMaterial = nullptr;
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrimitiveShelterManager : public AActor
{
    GENERATED_BODY()

public:
    APrimitiveShelterManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter System")
    FArch_ShelterConfiguration ShelterConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter System")
    TArray<FArch_ShelterComponent> WallComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter System")
    FArch_ShelterComponent RoofComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter System")
    FArch_ShelterComponent FirepitComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<UStaticMeshComponent*> SpawnedWalls;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    UStaticMeshComponent* SpawnedRoof;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    UStaticMeshComponent* SpawnedFirepit;

public:
    UFUNCTION(BlueprintCallable, Category = "Shelter Construction")
    void ConstructShelter();

    UFUNCTION(BlueprintCallable, Category = "Shelter Construction")
    void DestroyCurrentShelter();

    UFUNCTION(BlueprintCallable, Category = "Shelter Construction")
    void UpdateShelterConfiguration(const FArch_ShelterConfiguration& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Shelter Construction")
    bool LoadBiomeSpecificAssets(ESurvivalBiome BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Shelter Construction")
    void SpawnWallAtPosition(const FVector& Position, const FRotator& Rotation, int32 WallIndex);

    UFUNCTION(BlueprintCallable, Category = "Shelter Construction")
    void SpawnRoofStructure();

    UFUNCTION(BlueprintCallable, Category = "Shelter Construction")
    void SpawnFirepit();

    UFUNCTION(BlueprintCallable, Category = "Shelter Validation")
    bool ValidateShelterIntegrity() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter Validation")
    float CalculateShelterProtection() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void EditorConstructPreview();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void EditorClearPreview();

private:
    void SetupDefaultAssets();
    void CalculateWallPositions(TArray<FVector>& OutPositions, TArray<FRotator>& OutRotations) const;
    UStaticMeshComponent* CreateMeshComponent(const FArch_ShelterComponent& ComponentData, const FString& ComponentName);
    void ApplyBiomeWeathering(UStaticMeshComponent* MeshComponent, ESurvivalBiome BiomeType);
};