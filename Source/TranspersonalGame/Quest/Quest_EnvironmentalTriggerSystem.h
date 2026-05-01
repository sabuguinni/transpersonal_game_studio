#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerVolume.h"
#include "GameFramework/Actor.h"
#include "Quest_EnvironmentalTriggerSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_EnvironmentalTriggerType : uint8
{
    WaterSource     UMETA(DisplayName = "Water Source"),
    ForagingArea    UMETA(DisplayName = "Foraging Area"),
    CraftingSpot    UMETA(DisplayName = "Crafting Spot"),
    ShelterZone     UMETA(DisplayName = "Shelter Zone"),
    DangerZone      UMETA(DisplayName = "Danger Zone"),
    RestArea        UMETA(DisplayName = "Rest Area")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_EnvironmentalTriggerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Data")
    EQuest_EnvironmentalTriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Data")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Data")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Data")
    int32 QuestPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Data")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Data")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Data")
    FString TriggerMessage;

    FQuest_EnvironmentalTriggerData()
    {
        TriggerType = EQuest_EnvironmentalTriggerType::WaterSource;
        Location = FVector::ZeroVector;
        Radius = 200.0f;
        QuestPriority = 5;
        TimeLimit = 300.0f;
        bIsActive = true;
        TriggerMessage = TEXT("Environmental trigger activated");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_EnvironmentalTriggerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_EnvironmentalTriggerComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Trigger")
    FQuest_EnvironmentalTriggerData TriggerData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Trigger")
    TArray<AActor*> PlayersInTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Trigger")
    float ActivationCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Trigger")
    bool bRequiresSpecificItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Trigger")
    FString RequiredItemName;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Environmental Trigger")
    void ActivateTrigger(AActor* TriggeringActor);

    UFUNCTION(BlueprintCallable, Category = "Environmental Trigger")
    void DeactivateTrigger();

    UFUNCTION(BlueprintCallable, Category = "Environmental Trigger")
    bool IsPlayerInRange(AActor* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Trigger")
    void SetTriggerData(const FQuest_EnvironmentalTriggerData& NewTriggerData);

    UFUNCTION(BlueprintCallable, Category = "Environmental Trigger")
    FQuest_EnvironmentalTriggerData GetTriggerData() const { return TriggerData; }

    UFUNCTION(BlueprintImplementableEvent, Category = "Environmental Trigger")
    void OnTriggerActivated(AActor* TriggeringActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Environmental Trigger")
    void OnTriggerDeactivated();

private:
    float LastActivationTime;
    void CheckForPlayersInRange();
    bool ValidatePlayerRequirements(AActor* Player) const;
};

UCLASS()
class TRANSPERSONALGAME_API AQuest_EnvironmentalTriggerActor : public AActor
{
    GENERATED_BODY()

public:
    AQuest_EnvironmentalTriggerActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* TriggerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* TriggerCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UQuest_EnvironmentalTriggerComponent* EnvironmentalTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    class UMaterialInterface* ActiveMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    class UMaterialInterface* InactiveMaterial;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Environmental Trigger")
    void SetTriggerActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Environmental Trigger")
    void UpdateVisualState();

protected:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                              bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

#include "Quest_EnvironmentalTriggerSystem.generated.h"