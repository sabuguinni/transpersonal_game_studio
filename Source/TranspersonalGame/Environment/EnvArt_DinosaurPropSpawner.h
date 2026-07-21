#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "EnvArt_DinosaurPropSpawner.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_BiomeCoordinates
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float SpawnRadius;

    FEnvArt_BiomeCoordinates()
        : BiomeName(TEXT("Unknown"))
        , CenterLocation(FVector::ZeroVector)
        , SpawnRadius(15000.0f)
    {
    }

    FEnvArt_BiomeCoordinates(const FString& InBiomeName, const FVector& InCenter, float InRadius = 15000.0f)
        : BiomeName(InBiomeName)
        , CenterLocation(InCenter)
        , SpawnRadius(InRadius)
    {
    }
};

UENUM(BlueprintType)
enum class EEnvArt_PropType : uint8
{
    FallenLog       UMETA(DisplayName = "Fallen Log"),
    Boulder         UMETA(DisplayName = "Boulder"),
    Fern            UMETA(DisplayName = "Fern"),
    DeadTree        UMETA(DisplayName = "Dead Tree"),
    RockFormation   UMETA(DisplayName = "Rock Formation"),
    Mushroom        UMETA(DisplayName = "Mushroom"),
    Fossil          UMETA(DisplayName = "Fossil"),
    CrystalFormation UMETA(DisplayName = "Crystal Formation")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_DinosaurPropSpawner : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_DinosaurPropSpawner();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 PropsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    TArray<EEnvArt_PropType> PropTypesToSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    bool bAutoSpawnOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float MinDistanceBetweenProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugSpawning;

private:
    TArray<FEnvArt_BiomeCoordinates> BiomeCoordinates;
    TArray<AActor*> SpawnedProps;

    void InitializeBiomeCoordinates();
    FVector GetRandomLocationInBiome(const FEnvArt_BiomeCoordinates& Biome) const;
    bool IsValidSpawnLocation(const FVector& Location) const;
    AActor* SpawnPropAtLocation(EEnvArt_PropType PropType, const FVector& Location);
    UStaticMesh* GetMeshForPropType(EEnvArt_PropType PropType) const;

public:
    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void SpawnPropsInAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void SpawnPropsInBiome(const FString& BiomeName, int32 Count = 10);

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void ClearAllSpawnedProps();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogBiomeInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    int32 GetSpawnedPropsCount() const { return SpawnedProps.Num(); }
};