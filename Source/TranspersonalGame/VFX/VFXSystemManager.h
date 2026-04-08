#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "VFXSystemManager.generated.h"

/**
 * VFX System Manager - Gerencia todos os efeitos visuais do jogo
 * Implementa LOD chain a 3 níveis para performance otimizada
 * Baseado em Niagara VFX System do UE5
 */

UENUM(BlueprintType)
enum class EVFXQualityLevel : uint8
{
    Low = 0,        // Distância > 50m - Efeitos simplificados
    Medium = 1,     // Distância 20-50m - Efeitos padrão
    High = 2        // Distância < 20m - Efeitos completos
};

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environmental,  // Folhagem, poeira, vapor
    Creature,       // Respiração, pegadas, sangue
    Survival,       // Fogo, faíscas, decomposição
    Atmospheric,    // Chuva, névoa, distorção de calor
    Interactive     // Escavação, construção, domesticação
};

USTRUCT(BlueprintType)
struct FVFXDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSoftObjectPtr<UNiagaraSystem>> LODSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmissionIntensity = 1.0f;

    FVFXDefinition()
    {
        EffectName = TEXT("DefaultEffect");
        Category = EVFXCategory::Environmental;
        LODSystems.SetNum(3); // High, Medium, Low
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFXSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // VFX Library - Catálogo de todos os efeitos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Library")
    TMap<FString, FVFXDefinition> VFXLibrary;

    // Active VFX tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TMap<FString, UNiagaraComponent*> ActiveVFXComponents;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveVFX = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODUpdateFrequency = 0.1f; // 10 FPS para LOD updates

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float HighQualityDistance = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MediumQualityDistance = 50.0f;

private:
    float LODUpdateTimer = 0.0f;
    class APawn* PlayerPawn;

public:
    // Core VFX Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(const FString& EffectName, FVector Location, FRotator Rotation = FRotator::ZeroRotator, AActor* AttachToActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(const FString& InstanceID);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXByCategory(EVFXCategory Category);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    EVFXQualityLevel GetVFXQualityForDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void UpdateVFXLOD();

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SpawnFootstepEffect(FVector Location, bool bIsLarge = false, bool bIsMuddy = false);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SpawnFoliageDisturbance(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SpawnDustCloud(FVector Location, FVector Direction, float Intensity = 1.0f);

    // Creature VFX
    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void SpawnBreathEffect(AActor* Creature, bool bIsVisible = true, float Temperature = 20.0f);

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void SpawnBloodEffect(FVector Location, FVector Direction, bool bIsHeavyBleeding = false);

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void SpawnHeatDistortion(AActor* LargeCreature, float Intensity = 1.0f);

    // Survival VFX
    UFUNCTION(BlueprintCallable, Category = "Survival VFX")
    UNiagaraComponent* SpawnFireEffect(FVector Location, float Intensity = 1.0f, bool bIsCampfire = false);

    UFUNCTION(BlueprintCallable, Category = "Survival VFX")
    void SpawnSparksEffect(FVector Location, FVector Direction, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Survival VFX")
    void SpawnDecompositionEffect(AActor* OrganicObject, float Stage = 0.0f);

    // Atmospheric VFX
    UFUNCTION(BlueprintCallable, Category = "Atmospheric VFX")
    void SetRainIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric VFX")
    void SetFogDensity(float Density);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric VFX")
    void SpawnMorningMist(FVector Location, float Coverage = 1.0f);

    // Interactive VFX
    UFUNCTION(BlueprintCallable, Category = "Interactive VFX")
    void SpawnDiggingEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Interactive VFX")
    void SpawnCraftingEffect(FVector Location, FString CraftingType);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    void PreloadVFXAssets();

    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    void ClearInactiveVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    int32 GetActiveVFXCount();

private:
    void InitializeVFXLibrary();
    FString GenerateVFXInstanceID();
    void CleanupVFXComponent(UNiagaraComponent* Component);
    UNiagaraSystem* GetVFXSystemForLOD(const FString& EffectName, EVFXQualityLevel LOD);
};