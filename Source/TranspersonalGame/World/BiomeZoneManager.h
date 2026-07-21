#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeZoneManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    RockyBadlands UMETA(DisplayName = "Rocky Badlands"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    River       UMETA(DisplayName = "River"),
    COUNT       UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWater = false;
};

UCLASS(ClassGroup = (WorldGen), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeZoneManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeZoneManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Biome Query ----
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeZone GetBiomeZoneAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetVegetationDensityAt(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAt(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsLocationNearWater(const FVector& WorldLocation, float SearchRadius = 200.0f) const;

    // ---- Biome Registration ----
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeZone(const FWorld_BiomeZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void ClearAllBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome")
    void GenerateDefaultBiomes();

    // ---- Debug ----
    UFUNCTION(BlueprintCallable, Category = "Biome|Debug")
    void DrawBiomeBoundaries(float Duration = 5.0f) const;

    UFUNCTION(BlueprintPure, Category = "Biome")
    int32 GetBiomeCount() const { return BiomeZones.Num(); }

    UFUNCTION(BlueprintPure, Category = "Biome")
    FString GetBiomeNameAtLocation(const FVector& WorldLocation) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float BlendRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    bool bAutoGenerateOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    bool bDebugDrawEnabled = false;

private:
    float GetBiomeInfluence(const FWorld_BiomeZone& Zone, const FVector& Location) const;
    static FString BiomeTypeToString(EWorld_BiomeType Type);
};
