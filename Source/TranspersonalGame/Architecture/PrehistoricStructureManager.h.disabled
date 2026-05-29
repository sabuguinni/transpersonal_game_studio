#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "PrehistoricStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    CaveDwelling,
    WoodenShelter,
    StoneCircle,
    CliffDwelling,
    StoragePit,
    Watchtower,
    BridgeStructure,
    DefensiveWall
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::CaveDwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float DurabilityPercent = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 MaxOccupants = 4;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::CaveDwelling;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        DurabilityPercent = 100.0f;
        bIsOccupied = false;
        MaxOccupants = 4;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrehistoricStructureManager : public AActor
{
    GENERATED_BODY()

public:
    APrehistoricStructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* InteractionTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    TArray<class UStaticMesh*> StructureMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    TArray<class UMaterialInterface*> StructureMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    bool bAutoGenerateInterior = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    float WeatheringLevel = 0.3f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void InitializeStructure(EArch_StructureType InStructureType, FVector InLocation, FRotator InRotation);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void UpdateStructureDurability(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void GenerateInteriorProps();

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    bool CanOccupy() const;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SetOccupied(bool bOccupied);

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure Events")
    void OnStructureEntered(class APawn* EnteringPawn);

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure Events")
    void OnStructureExited(class APawn* ExitingPawn);

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure Events")
    void OnStructureDamaged(float DamageAmount, float NewDurability);

protected:
    UFUNCTION()
    void OnInteractionTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void SetupStructureMesh();
    void SetupInteriorElements();
    void CreateCaveDwelling();
    void CreateWoodenShelter();
    void CreateStoneCircle();
    void CreateCliffDwelling();

    TArray<class AActor*> InteriorProps;
    float LastWeatheringUpdate = 0.0f;
    bool bStructureInitialized = false;
};