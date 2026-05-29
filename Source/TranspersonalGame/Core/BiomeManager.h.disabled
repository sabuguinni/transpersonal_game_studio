#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Savanna     UMETA(DisplayName = "Savanna"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector Center = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float Radius = 20000.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float Temperature = 25.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float Humidity = 50.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBiomeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBiomeManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category="Biome")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    FBiomeData GetBiomeData(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    FVector GetRandomLocationInBiome(EBiomeType BiomeType) const;

private:
    TMap<EBiomeType, FBiomeData> BiomeData;

    void InitializeBiomeData();
    void SetupBiomeBoundaries();
    void UpdateBiomeEffects(float DeltaTime);
};
