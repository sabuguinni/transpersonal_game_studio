#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "../SharedTypes.h"
#include "PrehistoricShelterSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    Cave_Entrance      UMETA(DisplayName = "Cave Entrance"),
    Tree_Platform      UMETA(DisplayName = "Tree Platform"),
    Stone_Foundation   UMETA(DisplayName = "Stone Foundation"),
    Lean_To           UMETA(DisplayName = "Lean To"),
    Underground_Pit   UMETA(DisplayName = "Underground Pit")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::Cave_Entrance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float ProtectionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float TemperatureModifier = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasStorageSpace = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float DurabilityPoints = 100.0f;

    FArch_ShelterProperties()
    {
        ShelterType = EArch_ShelterType::Cave_Entrance;
        ProtectionLevel = 0.7f;
        TemperatureModifier = 5.0f;
        MaxOccupants = 2;
        bHasFirePit = true;
        bHasStorageSpace = true;
        DurabilityPoints = 100.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrehistoricShelter : public AActor
{
    GENERATED_BODY()

public:
    APrehistoricShelter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShelterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FirePitMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    FArch_ShelterProperties ShelterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter State")
    TArray<AActor*> CurrentOccupants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter State")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter State")
    float CurrentDurability = 100.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanEnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool EnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool ExitShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetProtectionLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetTemperatureBonus() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool HasFirePit() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void RepairShelter(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void DamageShelter(float DamageAmount);

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnShelterEntered(AActor* Actor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnShelterExited(AActor* Actor);

protected:
    UFUNCTION()
    void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void InitializeShelterMesh();
    void SetupShelterProperties();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPrehistoricShelterManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter Manager")
    APrehistoricShelter* CreateShelter(EArch_ShelterType ShelterType, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Shelter Manager")
    TArray<APrehistoricShelter*> GetAllShelters();

    UFUNCTION(BlueprintCallable, Category = "Shelter Manager")
    APrehistoricShelter* FindNearestShelter(FVector Location, float MaxDistance = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Shelter Manager")
    TArray<APrehistoricShelter*> GetSheltersByType(EArch_ShelterType ShelterType);

protected:
    UPROPERTY()
    TArray<APrehistoricShelter*> RegisteredShelters;

public:
    void RegisterShelter(APrehistoricShelter* Shelter);
    void UnregisterShelter(APrehistoricShelter* Shelter);
};