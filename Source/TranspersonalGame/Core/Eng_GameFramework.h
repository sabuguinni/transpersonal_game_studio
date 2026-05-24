#pragma once

#include "CoreMinimal.h"
#include "Engine/GameModeBase.h"
#include "Engine/GameStateBase.h"
#include "Engine/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Eng_GameFramework.generated.h"

/**
 * Engine Architect - Core Game Framework
 * Defines the essential game framework classes that all other systems depend on.
 * This is the foundation layer that must compile first before any other systems.
 */

// Forward declarations
class UEng_SurvivalComponent;
class UEng_DinosaurAIComponent;
class AEng_DinosaurBase;

/**
 * Core Game Mode - Manages the overall game rules and flow
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_TranspersonalGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AEng_TranspersonalGameMode();

protected:
    virtual void BeginPlay() override;
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    // Game configuration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Settings")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Settings")
    bool bEnablePermadeath = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Settings")
    int32 MaxPlayers = 4;

    // Survival settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survival")
    float HungerDecayRate = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survival")
    float ThirstDecayRate = 1.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survival")
    float FearDecayRate = 0.5f;

public:
    // Game mode functions
    UFUNCTION(BlueprintCallable, Category = "Game Mode")
    void SetTimeOfDay(ETimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Game Mode")
    void SetWeather(EWeatherType NewWeather);

    UFUNCTION(BlueprintPure, Category = "Game Mode")
    float GetGameTimeHours() const { return GameTimeHours; }

    UFUNCTION(BlueprintPure, Category = "Game Mode")
    ETimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Game Mode")
    EWeatherType GetCurrentWeather() const { return CurrentWeather; }

protected:
    // Game state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Game State")
    float GameTimeHours = 6.0f; // Start at dawn

    UPROPERTY(BlueprintReadOnly, Category = "Game State")
    ETimeOfDay CurrentTimeOfDay = ETimeOfDay::Dawn;

    UPROPERTY(BlueprintReadOnly, Category = "Game State")
    EWeatherType CurrentWeather = EWeatherType::Clear;

    // Internal functions
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherSystem(float DeltaTime);

private:
    FTimerHandle DayNightTimerHandle;
    FTimerHandle WeatherTimerHandle;
};

/**
 * Core Game State - Manages shared game state information
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_TranspersonalGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AEng_TranspersonalGameState();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // World state
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "World State")
    EBiomeType CurrentBiome = EBiomeType::Savanna;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "World State")
    EWeatherType CurrentWeather = EWeatherType::Clear;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "World State")
    ETimeOfDay CurrentTimeOfDay = ETimeOfDay::Dawn;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "World State")
    float WorldTemperature = 25.0f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "World State")
    float WorldHumidity = 60.0f;

    // Ecosystem state
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ecosystem")
    int32 TotalDinosaursAlive = 0;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ecosystem")
    int32 TotalPlayersAlive = 0;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ecosystem")
    TArray<EDinosaurSpecies> ActiveSpecies;

public:
    // Game state functions
    UFUNCTION(BlueprintCallable, Category = "Game State")
    void UpdateBiome(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Game State")
    void RegisterDinosaur(EDinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Game State")
    void UnregisterDinosaur(EDinosaurSpecies Species);

    UFUNCTION(BlueprintPure, Category = "Game State")
    int32 GetDinosaurCount(EDinosaurSpecies Species) const;

    UFUNCTION(BlueprintPure, Category = "Game State")
    bool IsBiomeSuitable(EDinosaurSpecies Species, EBiomeType Biome) const;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    // Species count tracking
    UPROPERTY()
    TMap<EDinosaurSpecies, int32> SpeciesCounts;

    void UpdateEcosystemMetrics();
};

/**
 * Core Player Controller - Handles player input and UI
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_TranspersonalPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AEng_TranspersonalPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaTime) override;

    // Input actions
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void StartJump();
    void StopJump();
    void StartRun();
    void StopRun();
    void StartCrouch();
    void StopCrouch();
    void InteractPressed();
    void InventoryPressed();

    // UI management
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<class UUserWidget> HUDWidgetClass;

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    class UUserWidget* HUDWidget;

public:
    // Controller functions
    UFUNCTION(BlueprintCallable, Category = "Player Controller")
    void ShowSurvivalStats();

    UFUNCTION(BlueprintCallable, Category = "Player Controller")
    void HideSurvivalStats();

    UFUNCTION(BlueprintCallable, Category = "Player Controller")
    void ToggleInventory();

    UFUNCTION(BlueprintImplementableEvent, Category = "Player Controller")
    void OnSurvivalStatChanged(ESurvivalStat StatType, float NewValue, float MaxValue);

private:
    // Input state
    bool bIsRunning = false;
    bool bIsCrouching = false;
    bool bInventoryOpen = false;

    // UI state
    bool bSurvivalStatsVisible = true;
};

/**
 * Survival Component - Manages character survival stats
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_SurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_SurvivalComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Survival stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    FSurvivalStats CurrentStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    FSurvivalStats MaxStats;

    // Decay rates (per second)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survival Config")
    float HungerDecayRate = 0.1f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survival Config")
    float ThirstDecayRate = 0.15f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survival Config")
    float StaminaRegenRate = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survival Config")
    float FearDecayRate = 0.5f;

public:
    // Survival functions
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyStat(ESurvivalStat StatType, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetStat(ESurvivalStat StatType, float Value);

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetStat(ESurvivalStat StatType) const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetStatPercentage(ESurvivalStat StatType) const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsStatCritical(ESurvivalStat StatType) const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsAlive() const;

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Survival")
    void OnStatChanged(ESurvivalStat StatType, float NewValue, float MaxValue);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival")
    void OnStatCritical(ESurvivalStat StatType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival")
    void OnDeath();

private:
    void UpdateSurvivalStats(float DeltaTime);
    void CheckCriticalStats();
    
    // Internal state
    bool bIsDead = false;
    float LastStatUpdateTime = 0.0f;
};

/**
 * Core Character - The player character with survival mechanics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_TranspersonalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AEng_TranspersonalCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UEng_SurvivalComponent* SurvivalComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* FollowCamera;

    // Movement settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float RunSpeed = 600.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float CrouchSpeed = 150.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float SwimSpeed = 200.0f;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    EMovementType CurrentMovementType = EMovementType::Walking;

    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsRunning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsFearing = false;

public:
    // Character functions
    UFUNCTION(BlueprintCallable, Category = "Character")
    void StartRunning();

    UFUNCTION(BlueprintCallable, Category = "Character")
    void StopRunning();

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetMovementType(EMovementType NewMovementType);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void AddFear(float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void Interact();

    UFUNCTION(BlueprintPure, Category = "Character")
    UEng_SurvivalComponent* GetSurvivalComponent() const { return SurvivalComponent; }

    UFUNCTION(BlueprintPure, Category = "Character")
    bool CanRun() const;

    UFUNCTION(BlueprintPure, Category = "Character")
    bool IsInDanger() const;

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Character")
    void OnMovementTypeChanged(EMovementType NewMovementType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Character")
    void OnFearLevelChanged(float FearLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "Character")
    void OnInteraction();

private:
    void UpdateMovementSpeed();
    void UpdateFearEffects();
    
    // Internal state
    float LastInteractionTime = 0.0f;
    const float InteractionCooldown = 0.5f;
};