#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "World_BiomeEcosystemManager.generated.h"

class AActor;
class UStaticMeshComponent;
class USkeletalMeshComponent;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeEcosystem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxDinosaurPopulation = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 CurrentDinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWaterSource = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector WaterSourceLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterSourceRadius = 5000.0f;

    FWorld_BiomeEcosystem()
    {
        DinosaurSpecies.Add("TRex");
        DinosaurSpecies.Add("Triceratops");
        DinosaurSpecies.Add("Parasaurolophus");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_DinosaurSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FString MeshAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    EBiomeType PreferredBiome = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 PackSize = 1;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UWorld_BiomeEcosystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UWorld_BiomeEcosystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    void CreateBiomeEcosystem(EBiomeType BiomeType, FVector CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    FWorld_BiomeEcosystem GetBiomeAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    EBiomeType GetBiomeTypeAtLocation(FVector Location);

    // Dinosaur Population Management
    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    void PopulateBiomeWithDinosaurs(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    AActor* SpawnDinosaurInBiome(const FString& SpeciesName, EBiomeType BiomeType, FVector PreferredLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    void SpawnDinosaurPack(const FString& SpeciesName, EBiomeType BiomeType, int32 PackSize, FVector CenterLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    int32 GetDinosaurCountInBiome(EBiomeType BiomeType);

    // Water System Management
    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    void CreateWaterSourceInBiome(EBiomeType BiomeType, FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    bool HasWaterSourceInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    FVector GetNearestWaterSource(FVector Location);

    // Ecosystem Balance
    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    void UpdateEcosystemBalance();

    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    void MigrateDinosaurs();

    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    bool CanSupportMoreDinosaurs(EBiomeType BiomeType);

    // Vegetation Management
    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    void UpdateVegetationDensity(EBiomeType BiomeType, float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Biome Ecosystem")
    float GetVegetationDensityAtLocation(FVector Location);

    // Debug and Editor Tools
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome Ecosystem")
    void DebugShowBiomeBoundaries();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome Ecosystem")
    void DebugPrintEcosystemStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome Ecosystem")
    void ClearAllBiomeActors();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Ecosystem", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_BiomeEcosystem> BiomeEcosystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Ecosystem", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_DinosaurSpawnData> DinosaurSpawnDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Ecosystem", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> SpawnedDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Ecosystem", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> WaterSources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Ecosystem", meta = (AllowPrivateAccess = "true"))
    float EcosystemUpdateInterval = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Ecosystem", meta = (AllowPrivateAccess = "true"))
    bool bAutoBalanceEcosystem = true;

private:
    void SetupDinosaurSpawnDatabase();
    FVector FindSuitableSpawnLocation(EBiomeType BiomeType, float MinDistanceFromOthers = 1000.0f);
    bool IsLocationInBiome(FVector Location, const FWorld_BiomeEcosystem& Biome);
    void RegisterSpawnedDinosaur(AActor* DinosaurActor, EBiomeType BiomeType);

    FTimerHandle EcosystemUpdateTimer;
};

#include "World_BiomeEcosystemManager.generated.h"