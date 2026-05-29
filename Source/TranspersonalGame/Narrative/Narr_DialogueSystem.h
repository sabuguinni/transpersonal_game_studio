#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/GameInstance.h"
#include "../SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    None = 0,
    LowHealth,
    Hunger,
    Thirst,
    DinosaurNearby,
    CombatWarning,
    Discovery,
    QuestComplete,
    FireDiscovered,
    ShelterBuilt,
    NightFalling,
    WeatherChange,
    TerritoryEntered
};

USTRUCT(BlueprintType)
struct FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    FNarr_DialogueEntry()
    {
        DialogueText = "";
        CharacterName = "Narrator";
        TriggerType = ENarr_DialogueTrigger::None;
        Priority = 1.0f;
        CooldownTime = 30.0f;
        bIsRepeatable = true;
        AudioPath = "";
    }
};

USTRUCT(BlueprintType)
struct FNarr_DialogueContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float PlayerHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float PlayerHunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float PlayerThirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bDinosaurNearby;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bInCombat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FString CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EWeatherType CurrentWeather;

    FNarr_DialogueContext()
    {
        PlayerHealth = 100.0f;
        PlayerHunger = 100.0f;
        PlayerThirst = 100.0f;
        bDinosaurNearby = false;
        bInCombat = false;
        CurrentBiome = "Savana";
        TimeOfDay = 12.0f;
        CurrentWeather = EWeatherType::Clear;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(ENarr_DialogueTrigger TriggerType, const FNarr_DialogueContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueEntry(const FNarr_DialogueEntry& Entry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanTriggerDialogue(ENarr_DialogueTrigger TriggerType) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDialogueCooldown(ENarr_DialogueTrigger TriggerType, float CooldownTime);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY()
    TMap<ENarr_DialogueTrigger, float> LastTriggerTimes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float GlobalCooldown;

    UPROPERTY()
    float LastDialogueTime;

private:
    FNarr_DialogueEntry* FindBestDialogue(ENarr_DialogueTrigger TriggerType, const FNarr_DialogueContext& Context);
    bool EvaluateDialogueConditions(const FNarr_DialogueEntry& Entry, const FNarr_DialogueContext& Context) const;
    void PlayDialogueAudio(const FNarr_DialogueEntry& Entry);
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_DialogueSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void RegisterDialogueComponent(UNarr_DialogueComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void UnregisterDialogueComponent(UNarr_DialogueComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void BroadcastDialogueTrigger(ENarr_DialogueTrigger TriggerType, const FNarr_DialogueContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void LoadDialogueDatabase();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void AddGlobalDialogue(const FNarr_DialogueEntry& Entry);

protected:
    UPROPERTY()
    TArray<UNarr_DialogueComponent*> RegisteredComponents;

    UPROPERTY()
    TArray<FNarr_DialogueEntry> GlobalDialogueDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxSimultaneousDialogues;

    UPROPERTY()
    int32 ActiveDialogueCount;

private:
    void InitializeDefaultDialogues();
    void LoadDialoguesFromConfig();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueTriggered, const FNarr_DialogueEntry&, DialogueEntry, const FNarr_DialogueContext&, Context);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue Trigger")
    void ActivateTrigger(AActor* TriggeringActor);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueTriggered OnDialogueTriggered;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Settings")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Settings")
    FNarr_DialogueEntry DialogueToTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Settings")
    bool bAutoTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Settings")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Settings")
    bool bRequireLineOfSight;

    UPROPERTY()
    class USphereComponent* TriggerSphere;

    UFUNCTION()
    void OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    bool CheckTriggerConditions(AActor* TriggeringActor);
    FNarr_DialogueContext BuildDialogueContext(AActor* TriggeringActor);
};