#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "../../SharedTypes.h"
#include "Eng_BiomeManager.generated.h"

/**
 * Engine Architect - BiomeManager
 * Core biome system architecture for the 5 biomes in the prehistoric world
 * Manages biome boundaries, environmental parameters, and spawning rules
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AEng_BiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === BIOME ARCHITECTURE ===
    
    /** Current active biome based on player location */
    UPROPERTY(BlueprintReadOnly, Category = "Biome System")
    EBiomeType CurrentBiome;
    
    /** Temperature in current biome (-20 to +50 Celsius) */
    UPROPERTY(BlueprintReadOnly, Category = "Biome System")
    float CurrentTemperature;
    
    /** Humidity percentage (0-100%) */
    UPROPERTY(BlueprintReadOnly, Category = "Biome System")
    float CurrentHumidity;
    
    /** Wind intensity (0-100) */
    UPROPERTY(BlueprintReadOnly, Category = "Biome System")
    float CurrentWindIntensity;
    
    // === BIOME BOUNDARIES (FIXED COORDINATES) ===
    
    /** Pantano (Swamp) boundaries */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Boundaries")
    FVector2D PantanoBounds_Min = FVector2D(-77500, -76500);
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Boundaries")
    FVector2D PantanoBounds_Max = FVector2D(-25000, -15000);
    
    /** Floresta (Forest) boundaries */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Boundaries")
    FVector2D FlorestaBounds_Min = FVector2D(-77500, 15000);
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Boundaries")
    FVector2D FlorestaBounds_Max = FVector2D(-15000, 76500);
    
    /** Savana boundaries */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Boundaries")
    FVector2D SavanaBounds_Min = FVector2D(-20000, -20000);
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Boundaries")
    FVector2D SavanaBounds_Max = FVector2D(20000, 20000);
    
    /** Deserto (Desert) boundaries */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Boundaries")
    FVector2D DesertoBounds_Min = FVector2D(25000, -30000);
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Boundaries")
    FVector2D DesertoBounds_Max = FVector2D(79500, 30000);
    
    /** Montanha Nevada (Snowy Mountain) boundaries */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Boundaries")
    FVector2D MontanhaBounds_Min = FVector2D(15000, 20000);
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Boundaries")
    FVector2D MontanhaBounds_Max = FVector2D(79500, 76500);
    
    // === CORE FUNCTIONS ===
    
    /** Determine biome type from world coordinates */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(FVector WorldLocation);
    
    /** Get biome center coordinates */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetBiomeCenter(EBiomeType BiomeType);
    
    /** Get random spawn location within biome */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetRandomLocationInBiome(EBiomeType BiomeType);
    
    /** Update environmental parameters based on current biome */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateEnvironmentalParameters(EBiomeType BiomeType);
    
    /** Check if location is valid for spawning specific actor types */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsValidSpawnLocation(FVector Location, EActorSpawnType SpawnType);
    
    /** Get biome-specific environmental multipliers */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FBiomeEnvironmentalData GetBiomeEnvironmentalData(EBiomeType BiomeType);

private:
    /** Reference to player character for biome tracking */
    UPROPERTY()
    class ATranspersonalCharacter* PlayerCharacter;
    
    /** Timer for environmental updates */
    float EnvironmentalUpdateTimer;
    
    /** Environmental update interval (seconds) */
    UPROPERTY(EditAnywhere, Category = "Performance")
    float UpdateInterval = 2.0f;
    
    /** Initialize biome boundaries and environmental data */
    void InitializeBiomeData();
    
    /** Update player's current biome */
    void UpdatePlayerBiome();
};