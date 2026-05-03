#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBuildIntegration, Log, All);

/**
 * Estrutura para definir zonas de biomas
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D XRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D YRange;

    FBuild_BiomeZone()
    {
        Name = TEXT("");
        Center = FVector::ZeroVector;
        XRange = FVector2D::ZeroVector;
        YRange = FVector2D::ZeroVector;
    }

    FBuild_BiomeZone(const FString& InName, const FVector& InCenter, const FVector2D& InXRange, const FVector2D& InYRange)
        : Name(InName), Center(InCenter), XRange(InXRange), YRange(InYRange)
    {
    }
};

/**
 * Manager responsável pela integração e validação de builds
 * Garante que todos os sistemas funcionam em conjunto
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

protected:
    virtual void BeginPlay() override;

    /** Estado actual da integração */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationState IntegrationState;

    /** Última vez que a validação foi executada */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration")
    FDateTime LastValidationTime;

    /** Intervalo entre validações automáticas (segundos) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ValidationIntervalSeconds;

    /** Lista de validações críticas que devem passar */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> CriticalValidations;

public:
    /**
     * Executa validação completa de integração
     */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void PerformIntegrationValidation();

    /**
     * Valida integridade dos módulos C++
     */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateModuleIntegrity();

    /**
     * Valida actores no MinPlayableMap
     */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateMapActors();

    /**
     * Valida distribuição de actores pelos biomas
     */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateBiomeDistribution();

    /**
     * Gera relatório de integração
     */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

    /**
     * Obtém estado actual da integração
     */
    UFUNCTION(BlueprintPure, Category = "Integration")
    EBuild_IntegrationState GetIntegrationState() const;

    /**
     * Verifica se é necessária nova validação
     */
    UFUNCTION(BlueprintPure, Category = "Integration")
    bool IsValidationRequired() const;
};