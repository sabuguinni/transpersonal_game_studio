#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

/**
 * Sistema de integração e validação de builds
 * Responsável por validar a integridade do projeto e coordenar sistemas
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

protected:
    virtual void BeginPlay() override;

public:
    /** Iniciar validação completa de integração */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void StartIntegrationValidation();

    /** Validar estrutura de ficheiros do projeto */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateFileStructure();

    /** Validar dependências de módulos */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateModuleDependencies();

    /** Validar actores no mapa actual */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateMapActors();

    /** Validar sistemas críticos */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateCriticalSystems();

    /** Gerar relatório completo de integração */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport GenerateIntegrationReport();

    /** Configurar intervalo de validação automática */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SetValidationInterval(float NewInterval);

    /** Activar/desactivar integração contínua */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void EnableContinuousIntegration(bool bEnable);

    /** Verificar se a integração está saudável */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Build Integration")
    bool IsIntegrationHealthy() const;

protected:
    /** Estado actual da integração */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration State")
    EBuild_IntegrationState IntegrationState;

    /** Última vez que a validação foi executada */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration State")
    FDateTime LastValidationTime;

    /** Intervalo entre validações automáticas (segundos) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings", meta = (ClampMin = "30.0"))
    float ValidationInterval;

    /** Se a validação automática está activada */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    bool bAutoValidationEnabled;

    /** Se a integração contínua está activada */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    bool bContinuousIntegration;

    /** Número máximo de erros antes de parar a build */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings", meta = (ClampMin = "1"))
    int32 MaxErrorsBeforeHalt;
};

/**
 * Actor de gestão de integração de builds
 * Pode ser colocado no mapa para monitorização contínua
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuildIntegrationActor : public AActor
{
    GENERATED_BODY()

public:
    ABuildIntegrationActor();

protected:
    virtual void BeginPlay() override;

public:
    /** Componente de gestão de integração */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBuildIntegrationManager* IntegrationManager;

    /** Executar validação manual */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build Integration")
    void RunManualValidation();

    /** Obter estado da integração */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Build Integration")
    EBuild_IntegrationState GetIntegrationState() const;

    /** Obter último relatório de integração */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport GetLastIntegrationReport() const;

protected:
    /** Último relatório gerado */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration State")
    FBuild_IntegrationReport LastReport;
};

#include "BuildIntegrationManager.generated.h"