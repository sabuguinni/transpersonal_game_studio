#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Arch_DwellingManager.generated.h"

UENUM(BlueprintType)
enum class EArch_DwellingType : uint8
{
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    StoneHut        UMETA(DisplayName = "Stone Hut"),
    RockShelter     UMETA(DisplayName = "Rock Shelter"),
    UndergroundDen  UMETA(DisplayName = "Underground Den"),
    TreeHouse       UMETA(DisplayName = "Tree House")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_DwellingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    EArch_DwellingType DwellingType = EArch_DwellingType::CaveEntrance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    float WeatherResistance = 0.8f;

    FArch_DwellingData()
    {
        DwellingType = EArch_DwellingType::CaveEntrance;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        StructuralIntegrity = 100.0f;
        bIsOccupied = false;
        MaxOccupants = 4;
        WeatherResistance = 0.8f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_DwellingActor : public AActor
{
    GENERATED_BODY()

public:
    AArch_DwellingActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* DwellingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* InteriorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Data")
    FArch_DwellingData DwellingInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Data")
    TArray<AActor*> CurrentOccupants;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    bool CanEnterDwelling(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    bool EnterDwelling(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    bool ExitDwelling(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    float GetStructuralIntegrity() const { return DwellingInfo.StructuralIntegrity; }

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void RepairDwelling(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void DamageDwelling(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    EArch_DwellingType GetDwellingType() const { return DwellingInfo.DwellingType; }

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    bool IsOccupied() const { return DwellingInfo.bIsOccupied; }

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    int32 GetOccupantCount() const { return CurrentOccupants.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    float GetWeatherProtection() const { return DwellingInfo.WeatherResistance; }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_DwellingManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_DwellingManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Management")
    TArray<AArch_DwellingActor*> ManagedDwellings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Management")
    int32 MaxDwellings = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Management")
    float DwellingSpawnRadius = 10000.0f;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Dwelling Management")
    AArch_DwellingActor* SpawnDwelling(EArch_DwellingType Type, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Dwelling Management")
    void RemoveDwelling(AArch_DwellingActor* Dwelling);

    UFUNCTION(BlueprintCallable, Category = "Dwelling Management")
    TArray<AArch_DwellingActor*> GetNearbyDwellings(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Dwelling Management")
    AArch_DwellingActor* FindNearestDwelling(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Dwelling Management")
    void GenerateRandomDwellings(int32 Count, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Dwelling Management")
    int32 GetDwellingCount() const { return ManagedDwellings.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Dwelling Management")
    void UpdateDwellingIntegrity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dwelling Management")
    void CleanupDestroyedDwellings();
};