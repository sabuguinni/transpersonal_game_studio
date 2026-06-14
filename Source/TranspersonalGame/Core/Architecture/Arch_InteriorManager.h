#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/TriggerVolume.h"
#include "Arch_InteriorManager.generated.h"

UENUM(BlueprintType)
enum class EArch_InteriorType : uint8
{
    Cave = 0,
    StoneShelter = 1,
    LogHut = 2,
    RockOverhang = 3,
    BuriedDwelling = 4
};

USTRUCT(BlueprintType)
struct FArch_InteriorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EArch_InteriorType InteriorType = EArch_InteriorType::Cave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float ComfortLevel = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float WarmthBonus = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasSleepingArea = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    int32 MaxOccupants = 1;

    FArch_InteriorData()
    {
        ComfortLevel = 50.0f;
        WarmthBonus = 25.0f;
        bHasFirePit = false;
        bHasSleepingArea = false;
        MaxOccupants = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_InteriorManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_InteriorManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* InteriorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* InteriorTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPointLightComponent* FireLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* FirePitMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Settings")
    FArch_InteriorData InteriorData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Settings")
    TArray<class UStaticMeshComponent*> FurnitureComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Settings")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Settings")
    TArray<AActor*> CurrentOccupants;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void SetupInteriorType(EArch_InteriorType NewType);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void AddFurniture(UStaticMesh* FurnitureMesh, FVector RelativeLocation, FRotator RelativeRotation);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void LightFirePit();

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void ExtinguishFirePit();

    UFUNCTION(BlueprintCallable, Category = "Interior")
    bool CanAccommodateOccupant() const;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void AddOccupant(AActor* NewOccupant);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void RemoveOccupant(AActor* OccupantToRemove);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    float GetComfortLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    float GetWarmthBonus() const;

    UFUNCTION()
    void OnInteriorEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteriorExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void UpdateFirePitVisuals();
    void SetupInteriorMesh();
    void ConfigureInteriorSettings();
};