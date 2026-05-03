#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

/**
 * Estrutura para resultados de validação de build
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuildValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString ValidationName;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bPassed = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> ErrorMessages;

    FBuildValidationResult()
    {
        ValidationName = TEXT("");
        bPassed = false;
        ErrorMessages.Empty();
    }
};

/**
 * Estados de integração do build
 */
UENUM(BlueprintType)
enum class EBuildIntegrationState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Validating  UMETA(DisplayName = "Validating"),
    Healthy     UMETA(DisplayName = "Healthy"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical")
};

/**
 * Manager responsável pela integração e validação de builds
 * Garante que todos os sistemas estão funcionais e integrados correctamente
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

protected:
    virtual void BeginPlay() override;

    /** Estado actual da integração */
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    EBuildIntegrationState IntegrationState;

    /** Resultados das validações */
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuildValidationResult> ValidationResults;

    /** Tempo da última validação */
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FDateTime LastValidationTime;

    /** Intervalo entre validações automáticas (segundos) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    float ValidationInterval;

    /** Lista de validações críticas que devem passar */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    TArray<FString> CriticalValidations;

public:
    /** Executa validação completa de integração */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ExecuteIntegrationValidation();

    /** Verifica se a integração está saudável */
    UFUNCTION(BlueprintPure, Category = "Build Integration")
    bool IsIntegrationHealthy() const;

    /** Obtém relatório detalhado de integração */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FString GetIntegrationReport() const;

    /** Força uma nova validação */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ForceValidation();

    /** Obtém estado actual da integração */
    UFUNCTION(BlueprintPure, Category = "Build Integration")
    EBuildIntegrationState GetIntegrationState() const { return IntegrationState; }

    /** Obtém resultados das validações */
    UFUNCTION(BlueprintPure, Category = "Build Integration")
    const TArray<FBuildValidationResult>& GetValidationResults() const { return ValidationResults; }

protected:
    /** Valida compilação de módulos */
    void ValidateModuleCompilation();

    /** Valida spawning de actores */
    void ValidateActorSpawning();

    /** Valida registo de componentes */
    void ValidateComponentRegistration();

    /** Valida carregamento de assets */
    void ValidateAssetLoading();

    /** Processa resultados das validações */
    void ProcessValidationResults();
};