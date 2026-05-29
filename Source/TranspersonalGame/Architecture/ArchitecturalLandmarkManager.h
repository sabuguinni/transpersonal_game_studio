#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "ArchitecturalLandmarkManager.generated.h"

UENUM(BlueprintType)
enum class EArch_LandmarkType : uint8
{
    StoneArch       UMETA(DisplayName = "Stone Arch"),
    StoneCircle     UMETA(DisplayName = "Stone Circle"),
    MegalithPillar  UMETA(DisplayName = "Megalith Pillar"),
    RuinedWall      UMETA(DisplayName = "Ruined Wall"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance")
};

USTRUCT(BlueprintType)
struct FArch_LandmarkData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    EArch_LandmarkType LandmarkType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    float WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FString BiomeName;

    FArch_LandmarkData()
    {
        LandmarkType = EArch_LandmarkType::StoneArch;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        WeatheringLevel = 0.5f;
        BiomeName = TEXT("Savana");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalLandmarkManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalLandmarkManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmarks")
    TArray<FArch_LandmarkData> LandmarkRegistry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    bool bAutoSpawnLandmarks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 MaxLandmarksPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WeatheredStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* MossyStoneMaterial;

public:
    UFUNCTION(BlueprintCallable, Category = "Landmarks")
    void SpawnLandmarkAtLocation(EArch_LandmarkType LandmarkType, FVector Location, FRotator Rotation, const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Landmarks")
    void SpawnLandmarksInBiome(const FString& BiomeName, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Landmarks")
    void RegisterLandmark(const FArch_LandmarkData& LandmarkData);

    UFUNCTION(BlueprintCallable, Category = "Landmarks")
    TArray<FArch_LandmarkData> GetLandmarksInBiome(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Landmarks", CallInEditor)
    void GenerateLandmarksForAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Landmarks")
    void ApplyWeatheringToLandmark(AActor* LandmarkActor, float WeatheringLevel);

private:
    void CreateStoneArchLandmark(FVector Location, FRotator Rotation, const FString& BiomeName);
    void CreateStoneCircleLandmark(FVector Location, FRotator Rotation, const FString& BiomeName);
    void CreateMegalithPillarLandmark(FVector Location, FRotator Rotation, const FString& BiomeName);
    void CreateRuinedWallLandmark(FVector Location, FRotator Rotation, const FString& BiomeName);
    void CreateCaveEntranceLandmark(FVector Location, FRotator Rotation, const FString& BiomeName);

    FVector GetRandomLocationInBiome(const FString& BiomeName) const;
    UStaticMesh* GetLandmarkMesh(EArch_LandmarkType LandmarkType) const;
    UMaterialInterface* GetWeatheredMaterial(float WeatheringLevel) const;
};