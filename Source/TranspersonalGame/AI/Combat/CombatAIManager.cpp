#include "CombatAIManager.h"
#include "DinosaurCharacter.h"
#include "CombatBehaviorComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACombatAIManager::ACombatAIManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS para otimização
    
    LastThreatUpdateTime = 0.0f;
}

void ACombatAIManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Encontrar todos os dinossauros existentes no nível
    TArray<AActor*> FoundDinosaurs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaurCharacter::StaticClass(), FoundDinosaurs);
    
    for (AActor* Actor : FoundDinosaurs)
    {
        if (ADinosaurCharacter* Dinosaur = Cast<ADinosaurCharacter>(Actor))
        {
            RegisterDinosaur(Dinosaur);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Registered %d dinosaurs"), ActiveDinosaurs.Num());
}

void ACombatAIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Atualizar decay das ameaças
    UpdateThreatDecay(DeltaTime);
    
    // Processar coordenação de combate
    if (CombatActiveDinosaurs.Num() > 0)
    {
        ProcessCombatCoordination();
    }
    
    // Atualizar comportamentos de matilha
    UpdatePackBehaviors();
}

void ACombatAIManager::RegisterDinosaur(ADinosaurCharacter* Dinosaur)
{
    if (Dinosaur && !ActiveDinosaurs.Contains(Dinosaur))
    {
        ActiveDinosaurs.Add(Dinosaur);
        
        // Configurar referência ao manager no dinossauro
        if (UCombatBehaviorComponent* CombatComp = Dinosaur->GetComponentByClass<UCombatBehaviorComponent>())
        {
            CombatComp->SetCombatManager(this);
        }
        
        UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Registered dinosaur %s"), *Dinosaur->GetName());
    }
}

void ACombatAIManager::UnregisterDinosaur(ADinosaurCharacter* Dinosaur)
{
    if (Dinosaur)
    {
        ActiveDinosaurs.Remove(Dinosaur);
        CombatActiveDinosaurs.Remove(Dinosaur);
        
        UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Unregistered dinosaur %s"), *Dinosaur->GetName());
    }
}

void ACombatAIManager::EnterCombatMode(ADinosaurCharacter* Dinosaur, AActor* Threat)
{
    if (Dinosaur && !CombatActiveDinosaurs.Contains(Dinosaur))
    {
        CombatActiveDinosaurs.Add(Dinosaur);
        
        // Atualizar nível de ameaça
        if (Threat)
        {
            UpdateThreatLevel(Threat, 100.0f, Threat->GetActorLocation());
        }
        
        // Alertar dinossauros próximos
        BroadcastAlert(Dinosaur->GetActorLocation(), Threat, 800.0f);
        
        UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: %s entered combat mode against %s"), 
               *Dinosaur->GetName(), Threat ? *Threat->GetName() : TEXT("Unknown"));
    }
}

void ACombatAIManager::ExitCombatMode(ADinosaurCharacter* Dinosaur)
{
    if (Dinosaur)
    {
        CombatActiveDinosaurs.Remove(Dinosaur);
        
        UE_LOG(LogTemp, Log, TEXT("CombatAIManager: %s exited combat mode"), *Dinosaur->GetName());
    }
}

void ACombatAIManager::UpdateThreatLevel(AActor* ThreatActor, float ThreatValue, FVector ThreatLocation)
{
    if (ThreatActor)
    {
        float* ExistingThreat = GlobalThreatMap.Find(ThreatActor);
        if (ExistingThreat)
        {
            *ExistingThreat = FMath::Max(*ExistingThreat, ThreatValue);
        }
        else
        {
            GlobalThreatMap.Add(ThreatActor, ThreatValue);
        }
        
        LastThreatUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

float ACombatAIManager::GetThreatLevel(AActor* ThreatActor) const
{
    if (const float* ThreatLevel = GlobalThreatMap.Find(ThreatActor))
    {
        return *ThreatLevel;
    }
    return 0.0f;
}

TArray<AActor*> ACombatAIManager::GetNearbyThreats(FVector Location, float Radius) const
{
    TArray<AActor*> NearbyThreats;
    
    for (const auto& ThreatPair : GlobalThreatMap)
    {
        if (ThreatPair.Value > 10.0f && ThreatPair.Key) // Apenas ameaças significativas
        {
            float Distance = FVector::Dist(Location, ThreatPair.Key->GetActorLocation());
            if (Distance <= Radius)
            {
                NearbyThreats.Add(ThreatPair.Key);
            }
        }
    }
    
    return NearbyThreats;
}

void ACombatAIManager::BroadcastAlert(FVector AlertLocation, AActor* ThreatActor, float AlertRadius)
{
    for (ADinosaurCharacter* Dinosaur : ActiveDinosaurs)
    {
        if (Dinosaur)
        {
            float Distance = FVector::Dist(AlertLocation, Dinosaur->GetActorLocation());
            if (Distance <= AlertRadius)
            {
                // Notificar dinossauro sobre a ameaça
                if (UCombatBehaviorComponent* CombatComp = Dinosaur->GetComponentByClass<UCombatBehaviorComponent>())
                {
                    CombatComp->ReceiveAlert(ThreatActor, AlertLocation, Distance);
                }
            }
        }
    }
    
    // Debug visual
    if (GetWorld())
    {
        DrawDebugSphere(GetWorld(), AlertLocation, AlertRadius, 12, FColor::Red, false, 2.0f);
    }
}

void ACombatAIManager::BroadcastPackHunt(ADinosaurCharacter* PackLeader, AActor* Target)
{
    if (!PackLeader || !Target) return;
    
    // Encontrar dinossauros da mesma espécie próximos
    for (ADinosaurCharacter* Dinosaur : ActiveDinosaurs)
    {
        if (Dinosaur && Dinosaur != PackLeader)
        {
            float Distance = FVector::Dist(PackLeader->GetActorLocation(), Dinosaur->GetActorLocation());
            if (Distance <= 1200.0f) // Raio de coordenação de matilha
            {
                // Verificar se é da mesma espécie (implementar sistema de espécies)
                if (UCombatBehaviorComponent* CombatComp = Dinosaur->GetComponentByClass<UCombatBehaviorComponent>())
                {
                    CombatComp->JoinPackHunt(PackLeader, Target);
                }
            }
        }
    }
}

FVector ACombatAIManager::GetFlankingPosition(ADinosaurCharacter* Attacker, AActor* Target, float PreferredDistance)
{
    if (!Attacker || !Target) return FVector::ZeroVector;
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector AttackerLocation = Attacker->GetActorLocation();
    
    // Calcular direção base
    FVector DirectionToTarget = (TargetLocation - AttackerLocation).GetSafeNormal();
    
    // Gerar posições de flanqueamento (90 graus à esquerda e direita)
    FVector LeftFlank = TargetLocation + FVector(-DirectionToTarget.Y, DirectionToTarget.X, 0) * PreferredDistance;
    FVector RightFlank = TargetLocation + FVector(DirectionToTarget.Y, -DirectionToTarget.X, 0) * PreferredDistance;
    
    // Escolher a posição mais próxima do atacante
    float LeftDistance = FVector::Dist(AttackerLocation, LeftFlank);
    float RightDistance = FVector::Dist(AttackerLocation, RightFlank);
    
    return (LeftDistance < RightDistance) ? LeftFlank : RightFlank;
}

bool ACombatAIManager::ShouldRetreat(ADinosaurCharacter* Dinosaur, AActor* Threat)
{
    if (!Dinosaur || !Threat) return false;
    
    // Implementar lógica de retirada baseada em:
    // - Saúde do dinossauro
    // - Tamanho relativo ao inimigo
    // - Número de aliados próximos
    // - Nível de ameaça global
    
    float ThreatLevel = GetThreatLevel(Threat);
    float DinosaurHealth = 100.0f; // Obter da implementação real
    
    // Retirar se saúde baixa ou ameaça muito alta
    return (DinosaurHealth < 30.0f) || (ThreatLevel > 150.0f);
}

ADinosaurCharacter* ACombatAIManager::GetNearestAlly(ADinosaurCharacter* Dinosaur, float MaxDistance)
{
    if (!Dinosaur) return nullptr;
    
    ADinosaurCharacter* NearestAlly = nullptr;
    float NearestDistance = MaxDistance;
    
    for (ADinosaurCharacter* PotentialAlly : ActiveDinosaurs)
    {
        if (PotentialAlly && PotentialAlly != Dinosaur)
        {
            float Distance = FVector::Dist(Dinosaur->GetActorLocation(), PotentialAlly->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestAlly = PotentialAlly;
            }
        }
    }
    
    return NearestAlly;
}

void ACombatAIManager::UpdateThreatDecay(float DeltaTime)
{
    TArray<AActor*> ThreatsToRemove;
    
    for (auto& ThreatPair : GlobalThreatMap)
    {
        ThreatPair.Value -= ThreatDecayRate * DeltaTime;
        
        if (ThreatPair.Value <= 0.0f || !IsValid(ThreatPair.Key))
        {
            ThreatsToRemove.Add(ThreatPair.Key);
        }
    }
    
    // Remover ameaças expiradas
    for (AActor* ThreatToRemove : ThreatsToRemove)
    {
        GlobalThreatMap.Remove(ThreatToRemove);
    }
}

void ACombatAIManager::ProcessCombatCoordination()
{
    // Implementar lógica de coordenação avançada
    // - Evitar que múltiplos dinossauros ataquem a mesma posição
    // - Coordenar ataques em grupo
    // - Gerenciar posicionamento tático
    
    for (int32 i = 0; i < CombatActiveDinosaurs.Num(); i++)
    {
        for (int32 j = i + 1; j < CombatActiveDinosaurs.Num(); j++)
        {
            ADinosaurCharacter* DinoA = CombatActiveDinosaurs[i];
            ADinosaurCharacter* DinoB = CombatActiveDinosaurs[j];
            
            if (DinoA && DinoB)
            {
                float Distance = FVector::Dist(DinoA->GetActorLocation(), DinoB->GetActorLocation());
                
                // Se muito próximos, um deve reposicionar
                if (Distance < 200.0f)
                {
                    if (UCombatBehaviorComponent* CombatComp = DinoB->GetComponentByClass<UCombatBehaviorComponent>())
                    {
                        CombatComp->RequestRepositioning();
                    }
                }
            }
        }
    }
}

void ACombatAIManager::UpdatePackBehaviors()
{
    // Implementar comportamentos de matilha
    // - Identificar líderes naturais
    // - Coordenar movimentos de grupo
    // - Sincronizar ataques
    
    // Esta funcionalidade será expandida com base no sistema de espécies
}