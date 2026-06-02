#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Eng_GameplayArchitect.generated.h"

UENUM(BlueprintType)
enum class EEng_GameplayLayer : uint8
{
    Core        UMETA(DisplayName = "Core Gameplay"),
    Movement    UMETA(DisplayName = "Character Movement"),
    Combat      UMETA(DisplayName = "Combat System"),
    Survival    UMETA(DisplayName = "Survival Mechanics"),
    World       UMETA(DisplayName = "World Interaction"),
    AI          UMETA(DisplayName = "AI Behavior"),
    Narrative   UMETA(DisplayName = "Quest & Narrative")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_GameplayMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    int32 ActivePlayers;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    int32 ActiveDinosaurs;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    float WorldLoadProgress;

    FEng_GameplayMetrics()
    {
        FrameTime = 0.0f;
        ActiveActors = 0;
        ActiveComponents = 0;
        MemoryUsageMB = 0.0f;
        ActivePlayers = 0;
        ActiveDinosaurs = 0;
        WorldLoadProgress = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_GameplayConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxWalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxRunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float JumpHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HungerDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float ThirstDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StaminaRegenRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BaseDamageMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 MaxSimultaneousDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls;

    FEng_GameplayConfig()
    {
        MaxWalkSpeed = 600.0f;
        MaxRunSpeed = 1200.0f;
        JumpHeight = 420.0f;
        HungerDecayRate = 1.0f;
        ThirstDecayRate = 1.5f;
        StaminaRegenRate = 10.0f;
        BaseDamageMultiplier = 1.0f;
        MaxSimultaneousDinosaurs = 150;
        MaxDrawCalls = 2000;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_GameplayArchitectSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void InitializeGameplayLayers();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void ValidateGameplayIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    FEng_GameplayMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void UpdateGameplayConfig(const FEng_GameplayConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    bool IsGameplayLayerActive(EEng_GameplayLayer Layer) const;

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void SetGameplayLayerActive(EEng_GameplayLayer Layer, bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeGameplayPerformance();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogGameplayStatus();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    FEng_GameplayConfig GameplayConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FEng_GameplayMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TMap<EEng_GameplayLayer, bool> LayerActiveStates;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastOptimizationTime;

private:
    void UpdateMetrics();
    void CheckPerformanceThresholds();
    void ValidateSystemIntegrity();
    bool IsPerformanceOptimal() const;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_GameplayArchitectComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_GameplayArchitectComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void RegisterWithArchitect();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void UnregisterFromArchitect();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    bool IsArchitectureValid() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetComponentPerformanceScore() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    EEng_GameplayLayer AssignedLayer;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsRegistered;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PerformanceScore;

private:
    void CalculatePerformanceScore();
    void ValidateComponentState();
};