#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "../SharedTypes.h"
#include "Arch_DwellingFoundationSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_FoundationType : uint8
{
    CircularStone UMETA(DisplayName = "Circular Stone"),
    RectangularStone UMETA(DisplayName = "Rectangular Stone"),
    NaturalRock UMETA(DisplayName = "Natural Rock"),
    EarthenMound UMETA(DisplayName = "Earthen Mound"),
    WoodPlatform UMETA(DisplayName = "Wood Platform"),
    CaveFloor UMETA(DisplayName = "Cave Floor")
};

USTRUCT(BlueprintType)
struct FArch_FoundationProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    EArch_FoundationType FoundationType = EArch_FoundationType::CircularStone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    float Diameter = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    float Height = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    bool bHasMossGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    bool bPartiallyBuried = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    int32 StoneBlockCount = 12;

    FArch_FoundationProperties()
    {
        FoundationType = EArch_FoundationType::CircularStone;
        Diameter = 400.0f;
        Height = 50.0f;
        WeatheringLevel = 0.5f;
        bHasMossGrowth = true;
        bPartiallyBuried = true;
        StoneBlockCount = 12;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_DwellingFoundationActor : public AActor
{
    GENERATED_BODY()

public:
    AArch_DwellingFoundationActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FoundationMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* FoundationBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation Properties")
    FArch_FoundationProperties FoundationConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation Properties")
    TArray<UStaticMesh*> StoneBlockMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation Properties")
    TArray<UMaterialInterface*> WeatheredMaterials;

public:
    UFUNCTION(BlueprintCallable, Category = "Foundation")
    void GenerateFoundation();

    UFUNCTION(BlueprintCallable, Category = "Foundation")
    void SetFoundationType(EArch_FoundationType NewType);

    UFUNCTION(BlueprintCallable, Category = "Foundation")
    void SetWeatheringLevel(float NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Foundation")
    bool IsLocationSuitableForDwelling(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Foundation")
    FVector GetFoundationCenter() const;

    UFUNCTION(BlueprintPure, Category = "Foundation")
    float GetFoundationRadius() const { return FoundationConfig.Diameter * 0.5f; }

    UFUNCTION(BlueprintPure, Category = "Foundation")
    EArch_FoundationType GetFoundationType() const { return FoundationConfig.FoundationType; }

private:
    void CreateCircularStoneFoundation();
    void CreateRectangularStoneFoundation();
    void CreateNaturalRockFoundation();
    void CreateEarthenMoundFoundation();
    void CreateWoodPlatformFoundation();
    void CreateCaveFloorFoundation();

    void ApplyWeathering();
    void AddMossGrowth();
    void ApplyBurialEffect();

    TArray<UStaticMeshComponent*> StoneComponents;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArch_DwellingFoundationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Foundation System")
    AArch_DwellingFoundationActor* CreateFoundationAtLocation(const FVector& Location, EArch_FoundationType Type);

    UFUNCTION(BlueprintCallable, Category = "Foundation System")
    TArray<AArch_DwellingFoundationActor*> FindFoundationsInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Foundation System")
    bool ValidateFoundationPlacement(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Foundation System")
    void RegisterFoundation(AArch_DwellingFoundationActor* Foundation);

    UFUNCTION(BlueprintCallable, Category = "Foundation System")
    void UnregisterFoundation(AArch_DwellingFoundationActor* Foundation);

protected:
    UPROPERTY()
    TArray<AArch_DwellingFoundationActor*> ActiveFoundations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation System")
    float MinimumFoundationDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation System")
    int32 MaxFoundationsPerArea = 5;

private:
    void CleanupInvalidFoundations();
    bool CheckTerrainSuitability(const FVector& Location, float Radius);
};