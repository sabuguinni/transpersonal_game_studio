#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"), 
    Desert      UMETA(DisplayName = "Desert"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Mountain    UMETA(DisplayName = "Mountain"),
    River       UMETA(DisplayName = "River"),
    Lake        UMETA(DisplayName = "Lake"),
    Volcanic    UMETA(DisplayName = "Volcanic")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Fertility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AnimalTypes;

    FWorld_BiomeData()
    {
        BiomeType = EWorld_BiomeType::Forest;
        BiomeName = TEXT("Default Biome");
        Temperature = 20.0f;
        Humidity = 50.0f;
        Fertility = 50.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_BiomeData> BiomeDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeTransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    int32 MaxBiomeRegions;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void GenerateBiomeRegions();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SpawnBiomeVegetation(EWorld_BiomeType BiomeType, const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float CalculateBiomeInfluence(const FVector& Location, EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System")
    void InitializeBiomeDatabase();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System")
    void CreateTestBiomeRegions();

private:
    void SetupDefaultBiomes();
    void CreateBiomeTransitions();
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius);
};