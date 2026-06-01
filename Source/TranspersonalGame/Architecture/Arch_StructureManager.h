#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Arch_StructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None = 0,
    Temple,
    Ruins,
    Pillar,
    Archway,
    Wall,
    Platform,
    Bridge,
    Tower
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString StructureName = "UnnamedStructure";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float Durability = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsRuined = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType = EBiomeType::Savanna;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::None;
        StructureName = "UnnamedStructure";
        Durability = 100.0f;
        WeatheringLevel = 0.0f;
        bIsRuined = false;
        BiomeType = EBiomeType::Savanna;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_StructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Settings")
    TArray<UStaticMesh*> StructureMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Settings")
    float WeatheringRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Settings")
    bool bAutoWeathering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Settings")
    float MaxViewDistance = 10000.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void InitializeStructure(EArch_StructureType Type, const FString& Name, EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SetStructureMesh(UStaticMesh* NewMesh);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SetRuinedState(bool bRuined);

    UFUNCTION(BlueprintPure, Category = "Structure Management")
    FArch_StructureData GetStructureData() const { return StructureData; }

    UFUNCTION(BlueprintPure, Category = "Structure Management")
    bool IsStructureRuined() const { return StructureData.bIsRuined; }

    UFUNCTION(BlueprintPure, Category = "Structure Management")
    float GetDurabilityPercent() const { return StructureData.Durability / 100.0f; }

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void RandomizeWeathering();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void ResetStructure();

protected:
    UFUNCTION()
    void UpdateStructureAppearance();

    UFUNCTION()
    void ProcessWeathering(float DeltaTime);

    UFUNCTION()
    void UpdateLOD();

private:
    float LastWeatheringUpdate = 0.0f;
    float WeatheringUpdateInterval = 5.0f;
    bool bIsInitialized = false;
};