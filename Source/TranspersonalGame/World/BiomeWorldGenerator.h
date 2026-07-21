#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeWorldGenerator.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Jungle    UMETA(DisplayName = "Jungle"),
    Savanna   UMETA(DisplayName = "Savanna"),
    Volcanic  UMETA(DisplayName = "Volcanic"),
    River     UMETA(DisplayName = "River"),
    Unknown   UMETA(DisplayName = "Unknown")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeWorldGenerator : public AActor
{
    GENERATED_BODY()

public:
    ABiomeWorldGenerator();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Biome query
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    FWorld_BiomeZone GetBiomeZoneAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetVegetationDensityAt(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetTemperatureAt(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetHumidityAt(const FVector& WorldLocation) const;

    // PCG generation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|PCG")
    void GenerateWorld();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|PCG")
    void ClearGeneratedActors();

    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    int32 GetGeneratedActorCount() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float WorldExtentX = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float WorldExtentY = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Config")
    int32 MaxVegetationActors = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Config")
    int32 RandomSeed = 42;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG|State")
    TArray<AActor*> GeneratedActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG|State")
    bool bWorldGenerated = false;

private:
    void InitDefaultBiomes();
    void SpawnVegetationForBiome(const FWorld_BiomeZone& Zone);
    void SpawnRiverForZone(const FWorld_BiomeZone& Zone);
    AActor* SpawnPrimitiveMesh(const FVector& Location, const FVector& Scale, const FString& Label);
};
