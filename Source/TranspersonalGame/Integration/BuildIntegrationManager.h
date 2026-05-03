#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "HAL/Platform.h"
#include "Misc/DateTime.h"
#include "TranspersonalGame/SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuildValidationComplete, bool, bSuccess, const TArray<FString>&, Errors);

/**
 * Build Integration Manager
 * Responsável por validar a integridade da build, verificar módulos carregados,
 * detectar problemas de compilação e coordenar a integração entre sistemas.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

protected:
    virtual void InitializeComponent() override;
    virtual void BeginPlay() override;

public:
    // === VALIDAÇÃO DE BUILD ===
    
    /**
     * Valida a integridade completa da build
     * @return true se a build está válida
     */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateBuildIntegrity();
    
    /**
     * Força uma revalidação completa
     */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ForceRevalidation();
    
    /**
     * Gera relatório detalhado do estado da build
     */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateBuildReport();
    
    // === INFORMAÇÕES DE BUILD ===
    
    /**
     * Obtém o status actual da build
     * @return String com status resumido
     */
    UFUNCTION(BlueprintPure, Category = "Build Integration")
    FString GetBuildStatus() const;
    
    /**
     * Define a versão da build
     */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SetBuildVersion(const FString& NewVersion);
    
    /**
     * Verifica se um módulo específico está carregado
     */
    UFUNCTION(BlueprintPure, Category = "Build Integration")
    bool IsModuleLoaded(const FString& ModuleName) const;

protected:
    // === VALIDAÇÃO INTERNA ===
    
    /**
     * Valida módulos core do jogo
     */
    bool ValidateCoreModules();
    
    /**
     * Valida dependências entre módulos
     */
    bool ValidateModuleDependencies();
    
    /**
     * Valida assets críticos
     */
    bool ValidateCriticalAssets();
    
    /**
     * Valida configuração do jogo
     */
    bool ValidateGameConfiguration();
    
    /**
     * Tenta corrigir automaticamente erros detectados
     */
    void AttemptAutoFix();

public:
    // === PROPRIEDADES EXPOSTAS ===
    
    /**
     * Versão actual da build
     */
    UPROPERTY(BlueprintReadOnly, Category = "Build Info")
    FString BuildVersion;
    
    /**
     * Timestamp da última build
     */
    UPROPERTY(BlueprintReadOnly, Category = "Build Info")
    FDateTime LastBuildTime;
    
    /**
     * Se a build actual é válida
     */
    UPROPERTY(BlueprintReadOnly, Category = "Build Info")
    bool bIsValidBuild;
    
    /**
     * Número total de módulos verificados
     */
    UPROPERTY(BlueprintReadOnly, Category = "Build Stats")
    int32 TotalModules;
    
    /**
     * Número de módulos compilados com sucesso
     */
    UPROPERTY(BlueprintReadOnly, Category = "Build Stats")
    int32 CompiledModules;
    
    /**
     * Número de módulos que falharam
     */
    UPROPERTY(BlueprintReadOnly, Category = "Build Stats")
    int32 FailedModules;
    
    /**
     * Lista de erros de validação encontrados
     */
    UPROPERTY(BlueprintReadOnly, Category = "Build Stats")
    TArray<FString> ValidationErrors;
    
    /**
     * Tempo da última validação em segundos
     */
    UPROPERTY(BlueprintReadOnly, Category = "Build Stats")
    float LastValidationTime;

protected:
    // === CONFIGURAÇÃO ===
    
    /**
     * Se deve validar automaticamente no startup
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Config")
    bool bValidateOnStartup;
    
    /**
     * Se deve tentar corrigir erros automaticamente
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Config")
    bool bAutoFixErrors;
    
    /**
     * Tempo máximo para validação em segundos
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Config", meta = (ClampMin = "1.0", ClampMax = "300.0"))
    float MaxValidationTime;

public:
    // === EVENTOS ===
    
    /**
     * Evento disparado quando validação completa
     */
    UPROPERTY(BlueprintAssignable, Category = "Build Events")
    FOnBuildValidationComplete OnBuildValidationComplete;
};