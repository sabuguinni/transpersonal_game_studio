#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "../SharedTypes.h"
#include "Arch_StructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StoneCircle     UMETA(DisplayName = "Stone Circle"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    RockFormation   UMETA(DisplayName = "Rock Formation"),
    AncientRuin     UMETA(DisplayName = "Ancient Ruin"),
    NaturalArch     UMETA(DisplayName = "Natural Arch")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::StoneCircle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType AssociatedBiome = EBiomeType::Forest;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::StoneCircle;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        WeatheringLevel = 0.5f;
        AssociatedBiome = EBiomeType::Forest;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Management")
    TArray<FArch_StructureData> ManagedStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Management")
    int32 MaxStructuresPerBiome = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Management")
    float MinDistanceBetweenStructures = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float WeatheringRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    bool bEnableWeatheringSystem = true;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    bool SpawnStructure(EArch_StructureType StructureType, FVector Location, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void RemoveStructure(int32 StructureIndex);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    TArray<FArch_StructureData> GetStructuresInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void ApplyWeathering(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Structure Management", CallInEditor = true)
    void GenerateStructuresForAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    bool IsLocationValidForStructure(FVector Location, EArch_StructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    FVector GetRandomLocationInBiome(EBiomeType BiomeType);

private:
    void CreateStoneCircle(FVector CenterLocation, EBiomeType BiomeType);
    void CreateCaveEntrance(FVector Location, EBiomeType BiomeType);
    void CreateRockFormation(FVector Location, EBiomeType BiomeType);
    void CreateAncientRuin(FVector Location, EBiomeType BiomeType);
    void CreateNaturalArch(FVector Location, EBiomeType BiomeType);

    AActor* SpawnStructureActor(FVector Location, FRotator Rotation, FVector Scale, const FString& Label);
    void ApplyWeatheringToActor(AActor* StructureActor, float WeatheringLevel);
};