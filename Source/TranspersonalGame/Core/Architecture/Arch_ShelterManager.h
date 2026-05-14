#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "Arch_ShelterManager.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    None = 0,
    CaveDwelling,
    ElevatedPlatform,
    StoneCircle,
    WorkshopArea,
    TemporaryLeanTo
};

USTRUCT(BlueprintType)
struct FArch_ShelterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WeatherProtection = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float TemperatureBonus = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasStorageArea = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasCraftingStation = false;

    FArch_ShelterData()
    {
        ShelterType = EArch_ShelterType::None;
        MaxOccupants = 4;
        WeatherProtection = 0.8f;
        TemperatureBonus = 5.0f;
        bHasFirePit = false;
        bHasStorageArea = false;
        bHasCraftingStation = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ShelterManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_ShelterManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShelterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    FArch_ShelterData ShelterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    TArray<FVector> OccupantPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    float InteractionRange = 300.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Shelter State")
    TArray<AActor*> CurrentOccupants;

    UPROPERTY(BlueprintReadOnly, Category = "Shelter State")
    bool bIsOccupied = false;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanEnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool EnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool ExitShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetWeatherProtection() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetTemperatureBonus() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool HasFirePit() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool HasStorageArea() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool HasCraftingStation() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void SetShelterType(EArch_ShelterType NewType);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void UpdateShelterMesh();

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    TArray<FVector> GetAvailablePositions();

private:
    void InitializeShelterData();
    void SetupOccupantPositions();
    FVector GetRandomPositionInShelter();
    bool IsPositionOccupied(const FVector& Position);
};