#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
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
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossGrowth;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::StoneCircle;
        Position = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        WeatheringLevel = 0.5f;
        bHasMossGrowth = true;
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
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> PrehistoricStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MinDistanceBetweenStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxStructuresPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* WeatheredStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* MossyStoneMaterial;

public:
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateStructuresInRadius(FVector CenterLocation, float Radius, int32 StructureCount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatheringToStructure(class AActor* StructureActor, float WeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AActor*> GetAllStructuresInRadius(FVector CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void GeneratePrehistoricStructures();

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void ClearAllStructures();

private:
    UFUNCTION()
    AActor* CreateStoneCircle(FVector Location, FRotator Rotation);

    UFUNCTION()
    AActor* CreateCaveEntrance(FVector Location, FRotator Rotation);

    UFUNCTION()
    AActor* CreateRockFormation(FVector Location, FRotator Rotation);

    UFUNCTION()
    AActor* CreateAncientRuin(FVector Location, FRotator Rotation);

    UFUNCTION()
    AActor* CreateNaturalArch(FVector Location, FRotator Rotation);

    UFUNCTION()
    void ApplyMaterialToActor(AActor* Actor, class UMaterialInterface* Material);

    UFUNCTION()
    bool IsValidSpawnLocation(FVector Location);

    TArray<AActor*> SpawnedStructures;
};