#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "ArchitecturalStructure.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Temple         UMETA(DisplayName = "Temple"),
    Ruins          UMETA(DisplayName = "Ruins"),
    Column         UMETA(DisplayName = "Column"),
    Archway        UMETA(DisplayName = "Archway"),
    Wall           UMETA(DisplayName = "Wall"),
    Platform       UMETA(DisplayName = "Platform"),
    Bridge         UMETA(DisplayName = "Bridge"),
    Shelter        UMETA(DisplayName = "Shelter")
};

UENUM(BlueprintType)
enum class EArch_StructureCondition : uint8
{
    Pristine       UMETA(DisplayName = "Pristine"),
    Weathered      UMETA(DisplayName = "Weathered"),
    Damaged        UMETA(DisplayName = "Damaged"),
    Ruined         UMETA(DisplayName = "Ruined"),
    Overgrown      UMETA(DisplayName = "Overgrown")
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::Ruins;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureCondition Condition = EArch_StructureCondition::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float Age = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsExplorable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasInterior = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString HistoricalDescription = TEXT("Ancient structure from the Cretaceous period");

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::Ruins;
        Condition = EArch_StructureCondition::Weathered;
        Age = 1000.0f;
        bIsExplorable = true;
        bHasInterior = false;
        HistoricalDescription = TEXT("Ancient structure from the Cretaceous period");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalStructure : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalStructure();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TArray<UStaticMeshComponent*> DetailMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bCanProvideShelte = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float ShelterRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bHasHiddenAreas = false;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetCondition(EArch_StructureCondition NewCondition);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    bool IsPlayerInShelter(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure")
    void OnPlayerEnterStructure(AActor* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure")
    void OnPlayerExitStructure(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    FString GetStructureDescription() const;

protected:
    UFUNCTION()
    void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void UpdateVisualCondition();

private:
    bool bPlayerInside = false;
    float LastWeatheringTime = 0.0f;
};