#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
// FIXME: Missing header - #include "../TranspersonalGameModule.h"
#include "TranspersonalGameInstance.generated.h"

/**
 * Game phases for the Transpersonal Game
 * Represents different states of the game lifecycle
 */
UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	MainMenu		UMETA(DisplayName = "Main Menu"),
	Loading			UMETA(DisplayName = "Loading"),
	InGame			UMETA(DisplayName = "In Game"),
	Paused			UMETA(DisplayName = "Paused")
};

/**
 * Biome types for world generation
 * Based on the Jurassic/Cretaceous setting from B1 concept
 */
UENUM(BlueprintType)
enum class ECore_BiomeType : uint8
{
	JurassicForest		UMETA(DisplayName = "Jurassic Forest"),
	CretaceousPlains	UMETA(DisplayName = "Cretaceous Plains"),
	TriassicDesert		UMETA(DisplayName = "Triassic Desert"),
	CoastalRegion		UMETA(DisplayName = "Coastal Region")
};

/**
 * Survival game settings structure
 * Configures core survival mechanics
 */
USTRUCT(BlueprintType)
struct FSurvivalSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	bool bEnableFear = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	bool bEnableHunger = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	bool bEnableThirst = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	bool bEnableFatigue = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	bool bPlayerIsPredator = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float FearIntensityMultiplier = 1.5f;
};

/**
 * Dinosaur behavior settings structure
 * Configures AI behavior and variation systems
 */
USTRUCT(BlueprintType)
struct FDinosaurSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs")
	bool bEnableIndependentBehavior = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs")
	bool bEnableUniqueVariations = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs")
	bool bEnableDomestication = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PhysicalVariationRange = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BehaviorVariationRange = 0.25f;
};

/**
 * World generation settings structure
 * Configures procedural world generation
 */
USTRUCT(BlueprintType)
struct FTranspersonalWorldSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
	ECore_BiomeType BiomeType = ECore_BiomeType::JurassicForest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
	bool bEnableDynamicWeather = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
	bool bEnableDayNightCycle = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float TimeScale = 1.0f;
};

/**
 * Main Game Instance class for the Transpersonal Game
 * 
 * Manages the overall game lifecycle, coordinates between subsystems,
 * and maintains game-wide state and configuration.
 * 
 * This class serves as the central coordinator for the 18-agent system,
 * ensuring proper initialization and communication between all game systems.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UTranspersonalGameInstance();

	// UGameInstance interface
	virtual void Init() override;
	virtual void Shutdown() override;

	/**
	 * Get current game phase
	 */
	UFUNCTION(BlueprintPure, Category = "Game State")
	EGamePhase GetCurrentGamePhase() const { return CurrentGamePhase; }

	/**
	 * Set game phase and trigger phase change events
	 */
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void SetGamePhase(EGamePhase NewPhase);

	/**
	 * Check if game is fully initialized
	 */
	UFUNCTION(BlueprintPure, Category = "Game State")
	bool IsGameInitialized() const { return bIsGameInitialized; }

	/**
	 * Get time since game started (in seconds)
	 */
	UFUNCTION(BlueprintPure, Category = "Game State")
	float GetGameStartTime() const { return GameStartTime; }

	// Game configuration accessors
	UFUNCTION(BlueprintPure, Category = "Configuration")
	const FSurvivalSettings& GetSurvivalSettings() const { return SurvivalSettings; }

	UFUNCTION(BlueprintPure, Category = "Configuration")
	const FDinosaurSettings& GetDinosaurSettings() const { return DinosaurSettings; }

	UFUNCTION(BlueprintPure, Category = "Configuration")
	const FTranspersonalWorldSettings& GetWorldSettings() const { return WorldSettings; }

protected:
	/**
	 * Initialize the Studio Director subsystem
	 * This coordinates the 18-agent production pipeline
	 */
	void InitializeStudioDirector();

	/**
	 * Setup game configuration based on B1 concept document
	 */
	void SetupGameConfiguration();

	/**
	 * Configure survival game mechanics
	 */
	void SetupSurvivalGameSettings();

	/**
	 * Configure dinosaur behavior and AI settings
	 */
	void SetupDinosaurBehaviorSettings();

	/**
	 * Configure world generation settings
	 */
	void SetupWorldSettings();

	/**
	 * Cleanup game systems during shutdown
	 */
	void CleanupGameSystems();

	/**
	 * Handle game phase transitions
	 */
	virtual void OnGamePhaseChanged(EGamePhase OldPhase, EGamePhase NewPhase);

private:
	// Game state
	UPROPERTY()
	bool bIsGameInitialized;

	UPROPERTY()
	float GameStartTime;

	UPROPERTY()
	EGamePhase CurrentGamePhase;

	// Game configuration
	UPROPERTY()
	FSurvivalSettings SurvivalSettings;

	UPROPERTY()
	FDinosaurSettings DinosaurSettings;

	UPROPERTY()
	FTranspersonalWorldSettings WorldSettings;
};