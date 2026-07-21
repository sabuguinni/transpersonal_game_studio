#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "Arch_BuildingManager.generated.h"

UENUM(BlueprintType)
enum class EArch_BuildingType : uint8
{
    StoneRuin,
    CaveEntrance,
    RockOverhang,
    WoodPlatform,
    BuriedShelter,
    AncientPillar,
    StoneCircle
};

USTRUCT(BlueprintType)
struct FArch_BuildingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    EArch_BuildingType BuildingType = EArch_BuildingType::StoneRuin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FVector Dimensions = FVector(500.0f, 500.0f, 300.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    bool bIsHabitable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float WeatherProtection = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<FVector> InteriorSpawnPoints;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_BuildingManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_BuildingManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* BuildingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteriorVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* ExteriorVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Data")
    FArch_BuildingData BuildingData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Settings")
    float TemperatureModifier = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Settings")
    float HumidityModifier = -10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Settings")
    float WindProtection = 0.7f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<AActor*> CurrentOccupants;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsPlayerInside = false;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Building")
    void InitializeBuilding(EArch_BuildingType Type, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Building")
    bool CanEnterBuilding(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Building")
    void OnActorEnterBuilding(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Building")
    void OnActorExitBuilding(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Building")
    float GetProtectionLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Building")
    FVector GetRandomInteriorPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Building")
    void SetBuildingMesh(UStaticMesh* NewMesh);

    UFUNCTION(BlueprintCallable, Category = "Building")
    void UpdateStructuralIntegrity(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Building")
    bool IsStructurallySafe() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Building")
    void OnPlayerEntered();

    UFUNCTION(BlueprintImplementableEvent, Category = "Building")
    void OnPlayerExited();

    UFUNCTION(BlueprintImplementableEvent, Category = "Building")
    void OnStructuralDamage(float NewIntegrity);

protected:
    UFUNCTION()
    void OnInteriorVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteriorVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void SetupBuildingMeshForType(EArch_BuildingType Type);
    void ConfigureBuildingData(EArch_BuildingType Type);
    void SetupCollisionVolumes();

private:
    float LastOccupancyCheck = 0.0f;
    const float OccupancyCheckInterval = 1.0f;
};

#include "Arch_BuildingManager.generated.h"